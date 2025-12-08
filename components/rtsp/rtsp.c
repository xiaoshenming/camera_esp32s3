#include "rtsp.h"
#include "wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/api.h"
#include "lwip/netdb.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static const char *TAG = "rtsp";

// RTSP客户端结构
typedef struct {
    int socket;
    bool connected;
    bool playing;
    char ip[16];
    uint16_t port;
    uint32_t session_id;
    int cseq;
} rtsp_client_t;

// RTSP服务器状态
static struct {
    bool initialized;
    bool running;
    int server_socket;
    rtsp_client_t clients[RTSP_MAX_CLIENTS];
    SemaphoreHandle_t mutex;
    TaskHandle_t server_task;
    TaskHandle_t stream_task;
    char stream_url[128];
    QueueHandle_t frame_queue;
} rtsp_server = {0};

// RTSP响应模板
static const char* RTSP_OPTIONS_RESPONSE = 
    "RTSP/1.0 200 OK\r\n"
    "CSeq: %d\r\n"
    "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n"
    "\r\n";

static const char* RTSP_DESCRIBE_RESPONSE = 
    "RTSP/1.0 200 OK\r\n"
    "CSeq: %d\r\n"
    "Content-Type: application/sdp\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "v=0\r\n"
    "o=- 0 0 IN IP4 %s\r\n"
    "s=ESP32 Camera Stream\r\n"
    "m=video 0 RTP/AVP 96\r\n"
    "a=rtpmap:96 RGB565/90000\r\n"
    "a=fmtp:96 width=640;height=480\r\n"
    "a=control:streamid=0\r\n";

static const char* RTSP_SETUP_RESPONSE = 
    "RTSP/1.0 200 OK\r\n"
    "CSeq: %d\r\n"
    "Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n"
    "Session: %u\r\n"
    "\r\n";

static const char* RTSP_PLAY_RESPONSE = 
    "RTSP/1.0 200 OK\r\n"
    "CSeq: %d\r\n"
    "Session: %u\r\n"
    "\r\n";

static const char* RTSP_TEARDOWN_RESPONSE = 
    "RTSP/1.0 200 OK\r\n"
    "CSeq: %d\r\n"
    "Session: %u\r\n"
    "\r\n";

// 解析RTSP请求
static bool parse_rtsp_request(const char* request, char* method, char* url, int* cseq, uint32_t* session_id) {
    char* line = strtok((char*)request, "\r\n");
    if (!line) return false;
    
    // 解析第一行: METHOD URL RTSP/1.0
    if (sscanf(line, "%s %s", method, url) != 2) {
        return false;
    }
    
    *cseq = 0;
    *session_id = 0;
    
    // 查找CSeq和Session
    while ((line = strtok(NULL, "\r\n")) != NULL) {
        if (strncmp(line, "CSeq:", 5) == 0) {
            sscanf(line, "CSeq: %d", cseq);
        } else if (strncmp(line, "Session:", 8) == 0) {
            sscanf(line, "Session: %lu", (unsigned long*)session_id);
        }
    }
    
    return true;
}

// 发送RTSP响应
static bool send_rtsp_response(int socket, const char* response, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, response);
    vsnprintf(buffer, sizeof(buffer), response, args);
    va_end(args);
    
    int len = strlen(buffer);
    int sent = send(socket, buffer, len, 0);
    return sent == len;
}

// 发送RTP包
static bool send_rtp_packet(int socket, const uint8_t* data, size_t len, uint32_t timestamp, uint16_t seq) {
    // RTP头部 (12字节)
    uint8_t rtp_header[12];
    rtp_header[0] = 0x80;  // Version=2, Padding=0, Extension=0, CSRC=0
    rtp_header[1] = 0x60;  // Marker=1, Payload=96 (dynamic RGB565)
    rtp_header[2] = (seq >> 8) & 0xFF;
    rtp_header[3] = seq & 0xFF;
    rtp_header[4] = (timestamp >> 24) & 0xFF;
    rtp_header[5] = (timestamp >> 16) & 0xFF;
    rtp_header[6] = (timestamp >> 8) & 0xFF;
    rtp_header[7] = timestamp & 0xFF;
    rtp_header[8] = 0x12;  // SSRC (随机)
    rtp_header[9] = 0x34;
    rtp_header[10] = 0x56;
    rtp_header[11] = 0x78;
    
    // TCP interleaved frame header (4字节)
    uint8_t interleaved[4];
    interleaved[0] = 0x24;  // '$'
    interleaved[1] = 0;      // Channel 0 (video)
    interleaved[2] = (len + 12) >> 8;
    interleaved[3] = (len + 12) & 0xFF;
    
    // 发送数据
    if (send(socket, interleaved, 4, 0) != 4) {
        return false;
    }
    
    if (send(socket, rtp_header, 12, 0) != 12) {
        return false;
    }
    
    if (send(socket, data, len, 0) != (int)len) {
        return false;
    }
    
    return true;
}

// 处理RTSP客户端
static void handle_rtsp_client(int client_socket, const char* client_ip) {
    char request[2048];
    char method[32], url[128];
    int cseq = 0;
    uint32_t session_id = 0;
    rtsp_client_t* client = NULL;
    
    ESP_LOGI(TAG, "Handling RTSP client from %s", client_ip);
    
    // 查找空闲客户端槽
    for (int i = 0; i < RTSP_MAX_CLIENTS; i++) {
        if (!rtsp_server.clients[i].connected) {
            client = &rtsp_server.clients[i];
            break;
        }
    }
    
    if (!client) {
        ESP_LOGW(TAG, "Maximum clients reached");
        close(client_socket);
        return;
    }
    
    // 初始化客户端
    client->socket = client_socket;
    client->connected = true;
    client->playing = false;
    client->session_id = 0;
    client->cseq = 0;
    strncpy(client->ip, client_ip, sizeof(client->ip) - 1);
    client->ip[sizeof(client->ip) - 1] = '\0';
    
    while (rtsp_server.running) {
        // 接收请求
        int len = recv(client_socket, request, sizeof(request) - 1, 0);
        if (len <= 0) {
            break;
        }
        request[len] = '\0';
        
        ESP_LOGD(TAG, "RTSP Request:\n%s", request);
        
        // 解析请求
        if (!parse_rtsp_request(request, method, url, &cseq, &session_id)) {
            ESP_LOGE(TAG, "Failed to parse RTSP request");
            break;
        }
        
        client->cseq = cseq;
        
        // 处理不同的RTSP方法
        if (strcmp(method, "OPTIONS") == 0) {
            send_rtsp_response(client_socket, RTSP_OPTIONS_RESPONSE, cseq);
        } else if (strcmp(method, "DESCRIBE") == 0) {
            char* ip = wifi_get_ip_address();
            char sdp[512];
            int sdp_len = snprintf(sdp, sizeof(sdp),
                "v=0\r\n"
                "o=- 0 0 IN IP4 %s\r\n"
                "s=ESP32 Camera Stream\r\n"
                "m=video 0 RTP/AVP 96\r\n"
                "a=rtpmap:96 RGB565/90000\r\n"
                "a=fmtp:96 width=640;height=480\r\n"
                "a=control:streamid=0\r\n", ip ? ip : "127.0.0.1");
            
            send_rtsp_response(client_socket, RTSP_DESCRIBE_RESPONSE, cseq, sdp_len, ip ? ip : "127.0.0.1");
        } else if (strcmp(method, "SETUP") == 0) {
            if (client->session_id == 0) {
                client->session_id = 12345678 + (client - rtsp_server.clients);
            }
            send_rtsp_response(client_socket, RTSP_SETUP_RESPONSE, cseq, client->session_id);
        } else if (strcmp(method, "PLAY") == 0) {
            client->playing = true;
            send_rtsp_response(client_socket, RTSP_PLAY_RESPONSE, cseq, client->session_id);
            ESP_LOGI(TAG, "Client %s started playing", client_ip);
        } else if (strcmp(method, "TEARDOWN") == 0) {
            send_rtsp_response(client_socket, RTSP_TEARDOWN_RESPONSE, cseq, client->session_id);
            break;
        }
    }
    
    // 清理客户端
    client->connected = false;
    client->playing = false;
    close(client_socket);
    ESP_LOGI(TAG, "RTSP client disconnected");
}

// RTSP流传输任务
static void rtsp_stream_task(void* arg) {
    camera_fb_t* frame;
    uint16_t sequence = 0;
    uint32_t timestamp = 0;
    TickType_t last_frame_time = 0;
    
    ESP_LOGI(TAG, "RTSP stream task started");
    
    while (rtsp_server.running) {
        // 等待帧数据
        if (xQueueReceive(rtsp_server.frame_queue, &frame, pdMS_TO_TICKS(100))) {
            if (!frame) {
                continue;
            }
            
            // 检查是否有活跃的客户端
            bool has_clients = false;
            for (int i = 0; i < RTSP_MAX_CLIENTS; i++) {
                if (rtsp_server.clients[i].connected && rtsp_server.clients[i].playing) {
                    has_clients = true;
                    break;
                }
            }
            
            if (!has_clients) {
                esp_camera_fb_return(frame);
                continue;
            }
            
            // 发送给所有播放中的客户端
            for (int i = 0; i < RTSP_MAX_CLIENTS; i++) {
                if (rtsp_server.clients[i].connected && rtsp_server.clients[i].playing) {
                    if (!send_rtp_packet(rtsp_server.clients[i].socket, frame->buf, frame->len, timestamp, sequence)) {
                        ESP_LOGW(TAG, "Failed to send frame to client %d", i);
                        rtsp_server.clients[i].playing = false;
                    }
                }
            }
            
            sequence++;
            timestamp += 90000 / 10;  // 假设10fps，时间戳单位90kHz
            last_frame_time = xTaskGetTickCount();
            
            esp_camera_fb_return(frame);
        }
        
        // 帧率控制
        TickType_t current_time = xTaskGetTickCount();
        if (current_time - last_frame_time < pdMS_TO_TICKS(100)) {  // 10fps
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    
    ESP_LOGI(TAG, "RTSP stream task ended");
}

// RTSP服务器任务
static void rtsp_server_task(void* arg) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    ESP_LOGI(TAG, "RTSP server task started");
    
    while (rtsp_server.running) {
        // 等待客户端连接
        int client_socket = accept(rtsp_server.server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (rtsp_server.running) {
                ESP_LOGE(TAG, "Accept failed: %s", strerror(errno));
            }
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        ESP_LOGI(TAG, "RTSP client connected from %s:%d", client_ip, ntohs(client_addr.sin_port));
        
        // 处理客户端请求
        handle_rtsp_client(client_socket, client_ip);
    }
    
    ESP_LOGI(TAG, "RTSP server task ended");
}

bool rtsp_init(void) {
    if (rtsp_server.initialized) {
        return true;
    }
    
    ESP_LOGI(TAG, "Initializing RTSP server...");
    
    // 创建互斥锁
    rtsp_server.mutex = xSemaphoreCreateMutex();
    if (!rtsp_server.mutex) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return false;
    }
    
    // 创建帧队列
    rtsp_server.frame_queue = xQueueCreate(2, sizeof(camera_fb_t*));
    if (!rtsp_server.frame_queue) {
        ESP_LOGE(TAG, "Failed to create frame queue");
        vSemaphoreDelete(rtsp_server.mutex);
        return false;
    }
    
    // 初始化客户端数组
    memset(rtsp_server.clients, 0, sizeof(rtsp_server.clients));
    
    rtsp_server.initialized = true;
    ESP_LOGI(TAG, "RTSP server initialized successfully");
    return true;
}

bool rtsp_start(void) {
    if (!rtsp_server.initialized) {
        ESP_LOGE(TAG, "RTSP server not initialized");
        return false;
    }
    
    if (rtsp_server.running) {
        ESP_LOGW(TAG, "RTSP server already running");
        return true;
    }
    
    ESP_LOGI(TAG, "Starting RTSP server on port %d", RTSP_PORT);
    
    // 创建服务器套接字
    rtsp_server.server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (rtsp_server.server_socket < 0) {
        ESP_LOGE(TAG, "Failed to create socket: %s", strerror(errno));
        return false;
    }
    
    // 设置套接字选项
    int opt = 1;
    setsockopt(rtsp_server.server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 绑定地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(RTSP_PORT);
    
    if (bind(rtsp_server.server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to bind socket: %s", strerror(errno));
        close(rtsp_server.server_socket);
        return false;
    }
    
    // 开始监听
    if (listen(rtsp_server.server_socket, RTSP_MAX_CLIENTS) < 0) {
        ESP_LOGE(TAG, "Failed to listen: %s", strerror(errno));
        close(rtsp_server.server_socket);
        return false;
    }
    
    // 生成流URL
    char* ip = wifi_get_ip_address();
    if (ip) {
        snprintf(rtsp_server.stream_url, sizeof(rtsp_server.stream_url), 
                "rtsp://%s:%d/%s", ip, RTSP_PORT, RTSP_PATH);
    } else {
        snprintf(rtsp_server.stream_url, sizeof(rtsp_server.stream_url), 
                "rtsp://127.0.0.1:%d/%s", RTSP_PORT, RTSP_PATH);
    }
    
    rtsp_server.running = true;
    
    // 创建流传输任务
    xTaskCreate(rtsp_stream_task, "rtsp_stream", 4096, NULL, 5, &rtsp_server.stream_task);
    
    // 创建服务器任务
    xTaskCreate(rtsp_server_task, "rtsp_server", 8192, NULL, 5, &rtsp_server.server_task);
    
    ESP_LOGI(TAG, "RTSP server started successfully");
    ESP_LOGI(TAG, "Stream URL: %s", rtsp_server.stream_url);
    
    return true;
}

bool rtsp_stop(void) {
    if (!rtsp_server.running) {
        return true;
    }
    
    ESP_LOGI(TAG, "Stopping RTSP server...");
    
    rtsp_server.running = false;
    
    // 关闭服务器套接字
    if (rtsp_server.server_socket >= 0) {
        close(rtsp_server.server_socket);
        rtsp_server.server_socket = -1;
    }
    
    // 等待任务结束
    if (rtsp_server.server_task) {
        vTaskDelete(rtsp_server.server_task);
        rtsp_server.server_task = NULL;
    }
    
    if (rtsp_server.stream_task) {
        vTaskDelete(rtsp_server.stream_task);
        rtsp_server.stream_task = NULL;
    }
    
    // 断开所有客户端
    for (int i = 0; i < RTSP_MAX_CLIENTS; i++) {
        if (rtsp_server.clients[i].socket >= 0) {
            close(rtsp_server.clients[i].socket);
            rtsp_server.clients[i].socket = -1;
        }
        rtsp_server.clients[i].connected = false;
        rtsp_server.clients[i].playing = false;
    }
    
    // 清理队列
    if (rtsp_server.frame_queue) {
        camera_fb_t* frame;
        while (xQueueReceive(rtsp_server.frame_queue, &frame, 0)) {
            if (frame) {
                esp_camera_fb_return(frame);
            }
        }
    }
    
    ESP_LOGI(TAG, "RTSP server stopped successfully");
    return true;
}

bool rtsp_is_running(void) {
    return rtsp_server.running;
}

char* rtsp_get_stream_url(void) {
    if (!rtsp_server.running) {
        return NULL;
    }
    return rtsp_server.stream_url;
}

void rtsp_send_frame(camera_fb_t* fb) {
    if (!rtsp_server.running || !fb) {
        return;
    }
    
    // 将帧发送到队列，不阻塞
    if (!xQueueSend(rtsp_server.frame_queue, &fb, 0)) {
        // 队列满了，释放帧
        esp_camera_fb_return(fb);
    }
}

int rtsp_get_client_count(void) {
    int count = 0;
    for (int i = 0; i < RTSP_MAX_CLIENTS; i++) {
        if (rtsp_server.clients[i].connected) {
            count++;
        }
    }
    return count;
}
