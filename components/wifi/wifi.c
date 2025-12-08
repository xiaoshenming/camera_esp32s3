#include "wifi.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/sockets.h"

static const char *TAG = "wifi";
static EventGroupHandle_t wifi_event_group;
static esp_netif_t *sta_netif = NULL;
static char ip_address[16] = {0};

// WiFi事件标志
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// WiFi事件处理函数
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                ESP_LOGI(TAG, "WiFi started, trying to connect...");
                break;
                
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WiFi disconnected, trying to reconnect...");
                esp_wifi_connect();
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                break;
                
            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                snprintf(ip_address, sizeof(ip_address), IPSTR, IP2STR(&event->ip_info.ip));
                ESP_LOGI(TAG, "Got IP address: %s", ip_address);
                xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
                break;
            }
                
            default:
                break;
        }
    }
}

bool wifi_init(void)
{
    ESP_LOGI(TAG, "Initializing WiFi...");
    
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 创建事件组
    wifi_event_group = xEventGroupCreate();
    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return false;
    }
    
    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_netif = esp_netif_create_default_wifi_sta();
    
    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 注册事件处理
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    // 设置WiFi模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    ESP_LOGI(TAG, "WiFi initialized successfully");
    return true;
}

bool wifi_connect(void)
{
    ESP_LOGI(TAG, "Connecting to WiFi SSID: %s", WIFI_SSID);
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi config: %s", esp_err_to_name(ret));
        return false;
    }
    
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WiFi: %s", esp_err_to_name(ret));
        return false;
    }
    
    return true;
}

char* wifi_get_ip_address(void)
{
    if (strlen(ip_address) == 0) {
        return NULL;
    }
    return ip_address;
}

bool wifi_is_connected(void)
{
    return (xEventGroupGetBits(wifi_event_group) & WIFI_CONNECTED_BIT) != 0;
}

bool wifi_wait_connection(int timeout_ms)
{
    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(timeout_ms));
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to WiFi successfully");
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to connect to WiFi within timeout");
        return false;
    }
}
