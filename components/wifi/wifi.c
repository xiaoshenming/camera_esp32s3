#include "wifi.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include <string.h>

static const char *TAG = "wifi";

static esp_netif_t *sta_netif = NULL;
static int udp_socket = -1;
static struct sockaddr_in broadcast_addr;
static SemaphoreHandle_t wifi_mutex = NULL;
static bool wifi_connected = false;
static uint16_t current_frame_id = 0;

// 统计信息变量
static uint32_t stats_frames_sent = 0;
static uint32_t stats_packets_sent = 0;
static uint32_t stats_bytes_sent = 0;
static float stats_fps = 0.0f;
static uint32_t stats_last_time = 0;

// 更新统计信息的函数（在wifi_send_camera_frame中调用）
static void wifi_update_stats(uint16_t packets, size_t bytes)
{
    stats_packets_sent += packets;
    stats_bytes_sent += bytes;
    stats_frames_sent++;
    
    // 计算FPS
    uint32_t current_time = xTaskGetTickCount();
    if (stats_last_time == 0) {
        stats_last_time = current_time;
    } else {
        float elapsed_time = (current_time - stats_last_time) / 1000.0f;
        if (elapsed_time >= 1.0f) {
            stats_fps = stats_frames_sent / elapsed_time;
            stats_last_time = current_time;
            stats_frames_sent = 0;
        }
    }
}

// WiFi事件处理函数
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi started, connecting to AP...");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WiFi disconnected, trying to reconnect...");
                wifi_connected = false;
                esp_wifi_connect();
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP:
                {
                    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                    ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
                    wifi_connected = true;
                    
                    // 设置目标地址 - 直接发送到192.168.1.113（接收端）
                    broadcast_addr.sin_family = AF_INET;
                    broadcast_addr.sin_port = htons(UDP_PORT);
                    // 直接使用接收端IP地址：192.168.1.113
                    broadcast_addr.sin_addr.s_addr = inet_addr("192.168.1.113");
                    
                    ESP_LOGI(TAG, "Target address: 192.168.1.113:%d", UDP_PORT);
                }
                break;
            default:
                break;
        }
    }
}

bool wifi_init_sta(const char* ssid, const char* password)
{
    ESP_LOGI(TAG, "Initializing WiFi in STA mode...");
    
    esp_err_t ret;
    
    // 创建互斥锁
    wifi_mutex = xSemaphoreCreateMutex();
    if (!wifi_mutex) {
        ESP_LOGE(TAG, "Failed to create WiFi mutex");
        return false;
    }
    
    // 初始化NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "Erasing NVS flash...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return false;
    }
    
    // 初始化网络接口
    ret = esp_netif_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize netif: %s", esp_err_to_name(ret));
        return false;
    }
    
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to create event loop: %s", esp_err_to_name(ret));
        return false;
    }
    
    sta_netif = esp_netif_create_default_wifi_sta();
    if (!sta_netif) {
        ESP_LOGE(TAG, "Failed to create default wifi sta");
        return false;
    }
    
    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
        return false;
    }
    
    // 注册事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                             ESP_EVENT_ANY_ID,
                                             &wifi_event_handler,
                                             NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                             ESP_EVENT_ANY_ID,
                                             &wifi_event_handler,
                                             NULL));
    
    // 配置WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi initialization completed");
    return true;
}

bool wifi_udp_broadcast_init(uint16_t port)
{
    if (udp_socket >= 0) {
        close(udp_socket);
    }
    
    // 创建UDP socket
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        ESP_LOGE(TAG, "Failed to create UDP socket: %s", strerror(errno));
        return false;
    }
    
    // 设置广播权限
    int broadcast = 1;
    if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        ESP_LOGE(TAG, "Failed to set broadcast permission: %s", strerror(errno));
        close(udp_socket);
        udp_socket = -1;
        return false;
    }
    
    // ESP32 WiFi协议栈不支持设置SO_SNDBUF，跳过此设置
    ESP_LOGI(TAG, "Using default UDP send buffer size");
    
    // 设置超时
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000; // 10ms
    if (setsockopt(udp_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        ESP_LOGW(TAG, "Failed to set send timeout: %s", strerror(errno));
    }
    
    ESP_LOGI(TAG, "UDP broadcast initialized on port %d", port);
    return true;
}

int wifi_udp_send(const void* data, size_t len)
{
    if (udp_socket < 0 || !wifi_connected) {
        ESP_LOGE(TAG, "UDP send failed: socket=%d, connected=%d", udp_socket, wifi_connected);
        return -1;
    }
    
    if (xSemaphoreTake(wifi_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
        ESP_LOGE(TAG, "UDP send failed: mutex timeout");
        return -1;
    }
    
    int sent = sendto(udp_socket, data, len, 0, 
                      (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    
    xSemaphoreGive(wifi_mutex);
    
    if (sent < 0) {
        ESP_LOGE(TAG, "UDP send failed: %s (errno=%d)", strerror(errno), errno);
        ESP_LOGE(TAG, "Socket state: fd=%d, connected=%d", udp_socket, wifi_connected);
        ESP_LOGE(TAG, "Target: %d.%d.%d.%d:%d", 
                 (broadcast_addr.sin_addr.s_addr >> 0) & 0xFF,
                 (broadcast_addr.sin_addr.s_addr >> 8) & 0xFF,
                 (broadcast_addr.sin_addr.s_addr >> 16) & 0xFF,
                 (broadcast_addr.sin_addr.s_addr >> 24) & 0xFF,
                 ntohs(broadcast_addr.sin_port));
    } else {
        ESP_LOGD(TAG, "UDP send success: %d bytes", sent);
    }
    
    return sent;
}

bool wifi_is_connected(void)
{
    return wifi_connected;
}

char* wifi_get_local_ip(void)
{
    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(sta_netif, &ip_info) != ESP_OK) {
        return NULL;
    }
    
    char* ip_str = malloc(16);
    if (!ip_str) {
        return NULL;
    }
    
    snprintf(ip_str, 16, IPSTR, IP2STR(&ip_info.ip));
    return ip_str;
}

bool wifi_send_camera_frame(const uint8_t* frame_data, size_t frame_size, uint16_t frame_id)
{
    if (!frame_data || frame_size == 0 || udp_socket < 0) {
        return false;
    }
    
    // 检查帧大小是否超过限制
    if (frame_size > MAX_FRAME_SIZE) {
        ESP_LOGW(TAG, "Frame too large: %d bytes (max: %d)", frame_size, MAX_FRAME_SIZE);
        return false;
    }
    
    // 准备完整的UDP数据包
    static uint8_t packet_buffer[UDP_PACKET_SIZE];
    
    // 使用结构体构建包头以确保正确的字节序和对齐
    udp_frame_t* frame = (udp_frame_t*)packet_buffer;
    frame->magic = UDP_MAGIC_NUMBER;  // 0x5056
    frame->width = 160;               // QQVGA宽度
    frame->height = 120;              // QQVGA高度
    
    // 复制图像数据
    memcpy(frame->data, frame_data, frame_size);
    
    // 发送完整帧
    size_t total_size = sizeof(udp_frame_t) - 1 + frame_size;  // 头部(6字节) + 图像数据
    int sent = wifi_udp_send(packet_buffer, total_size);
    
    if (sent < 0) {
        ESP_LOGW(TAG, "Failed to send frame %d", frame_id);
        return false;
    }
    
    // 更新统计信息
    wifi_update_stats(1, frame_size);
    
    return true;
}

bool wifi_get_info(wifi_info_t* info)
{
    if (!info || !wifi_connected) {
        return false;
    }
    
    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(sta_netif, &ip_info) != ESP_OK) {
        return false;
    }
    
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK) {
        return false;
    }
    
    // 获取当前连接的AP信息
    wifi_config_t config;
    if (esp_wifi_get_config(WIFI_IF_STA, &config) != ESP_OK) {
        return false;
    }
    
    strncpy(info->ssid, (char*)config.sta.ssid, sizeof(info->ssid) - 1);
    info->ssid[sizeof(info->ssid) - 1] = '\0';
    info->ip = ip_info.ip.addr;
    info->channel = ap_info.primary;
    
    return true;
}

bool wifi_get_stats(uint32_t* frames_sent, uint32_t* packets_sent, uint32_t* bytes_sent, float* fps)
{
    if (!frames_sent || !packets_sent || !bytes_sent || !fps) {
        return false;
    }
    
    *frames_sent = stats_frames_sent;
    *packets_sent = stats_packets_sent;
    *bytes_sent = stats_bytes_sent;
    *fps = stats_fps;
    
    return true;
}
