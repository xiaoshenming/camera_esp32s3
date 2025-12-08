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

static const char *TAG = "rtsp";

// RTSP客户端结构
typedef struct {
    int socket;
    bool connected;
    bool playing;
    char ip[16];
    uint16_t port;
} rtsp_client_t;

// RTSP服务器状态
static struct {
    bool initialized;
    bool running;
    int server_socket;
    rtsp_client_t clients[RTSP_MAX_CLIENTS];
    SemaphoreHandle_t mutex;
    TaskHandle_t server_task;
    char stream_url[128];
} rtsp_server = {0};

// MJPEG边界标记
#define MJPEG_BOUNDARY "myboundary"
#define MJPEG_HEADER "Content-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n"

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
    "a=rtpmap:96 JPEG/90000\r\n"
    "a=control:streamid=0\r\n";

static const char* RTSP_SETUP_RESPONSE = 
    "RTSP/1.0 200 OK\r\n"
    "CSeq: %d\r\n"
    "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n"
    "Session: 12345678\r\n"
    "\r\n";

static const char* RTSP_PLAY_RESPONSE = 
    "RTSP/1.0 200 OK\r\n"
    "CSeq: %d\r\n"
    "Session: 12345678\r\n"
    "\r\n";

static const char* RTSP_TEARDOWN_RESPONSE = 
    "RTSP/1.0 200 OK\r\n"
    "CSeq: %d\r\n"
    "Session: 12345678\r\n"
    "\r\n";

// 解析RTSP请求
static bool parse_rtsp_request(const char* request, char* method, char* url, int* cseq) {
    char* line = strtok((char*)request, "\r\n");
    if (!line) return false;
    
    // 解析第一行: METHOD URL RTSP/1.0
    if (sscanf(line, "%s %s", method, url) != 2) {
        return false;
    }
    
    // 查找CSeq
    while ((line = strtok(NULL, "\r\n")) != NULL) {
        if (strncmp(line, "CSeq:", 5) == 0) {
            sscanf(line, "CSeq: %d", cseq);
            return true;
        }
    }
    
    return false;
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

// 处理RTSP客户端
static void handle_rtsp_client(int client_socket, const char* client_ip) {
    char request[2048];
    char method[32], url[128];
    int cseq = 0;
    
    ESP_LOGI(TAG, "Handling RTSP client from %s", client_ip);
    
    while (rtsp_server.running) {
        // 接收请求
        int len = recv(client_socket, request, sizeof(request) - 1, 0);
        if (len <= 0) {
            break;
        }
        request[len] = '\0';
        
        ESP_LOGD(TAG, "RTSP Request:\n%s", request);
        
        // 解析请求
        if (!parse_rtsp_request(request, method, url, &cseq)) {
            ESP_LOGE(TAG, "Failed to parse RTSP request");
            break;
        }
        
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
                "a=rtpmap:96 JPEG/90000\r\n"
                "a=control:streamid=0\r\n", ip ? ip : "127.0.0.1");
            
            send_rtsp_response(client_socket, RTSP_DESCRIBE_RESPONSE, cseq, sdp_len, ip ? ip : "127.0.0.1");
        } else if (strcmp(method, "SETUP") == 0) {
            // 简化处理，假设客户端端口为5000-5001
            send_rtsp_response(client_socket, RTSP_SETUP_RESPONSE, cseq, 5000, 5001);
        } else if (strcmp(method, "PLAY") == 0) {
            send_rtsp_response(client_socket, RTSP_PLAY_RESPONSE, cseq);
            // 开始发送MJPEG流
            break;
        } else if (strcmp(method, "TEARDOWN") == 0) {
            send_rtsp_response(client_socket, RTSP_TEARDOWN_RESPONSE, cseq);
            break;
        }
    }
    
    close(client_socket);
    ESP_LOGI(TAG, "RTSP client disconnected");
}

// RTSP服务器任务
static void rtsp_server_task(void* arg) {
    struct sockaddr_in server_addr, client_addr;
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
    
    // 创建服务器任务
    xTaskCreate(rtsp_server_task, "rtsp_server", 4096, NULL, 5, &rtsp_server.server_task);
    
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
    
    // 断开所有客户端
    for (int i = 0; i < RTSP_MAX_CLIENTS; i++) {
        if (rtsp_server.clients[i].socket >= 0) {
            close(rtsp_server.clients[i].socket);
            rtsp_server.clients[i].socket = -1;
        }
        rtsp_server.clients[i].connected = false;
        rtsp_server.clients[i].playing = false;
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
    // 简化实现，实际RTSP流需要更复杂的处理
    // 这里只是占位符，真正的RTSP实现需要RTP协议
    ESP_LOGD(TAG, "Frame sent to RTSP clients: %d bytes", fb ? fb->len : 0);
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
