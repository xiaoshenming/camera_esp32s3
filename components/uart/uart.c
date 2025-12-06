#include "uart.h"
#include "esp_log.h"
#include "driver/uart.h"

static const char *TAG = "uart";

#define UART_NUM UART_NUM_0
#define UART_BUF_SIZE 1024

bool uart_init(void)
{
    ESP_LOGI(TAG, "Initializing UART...");
    
    // 配置UART参数
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // 安装UART驱动
    esp_err_t ret = uart_driver_install(UART_NUM, UART_BUF_SIZE, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(ret));
        return false;
    }
    
    // 配置UART参数
    ret = uart_param_config(UART_NUM, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART parameters: %s", esp_err_to_name(ret));
        return false;
    }
    
    // 设置UART引脚（使用默认引脚）
    ret = uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins: %s", esp_err_to_name(ret));
        return false;
    }
    
    ESP_LOGI(TAG, "UART initialized successfully");
    return true;
}

bool uart_send_string(const char* data)
{
    if (data == NULL) {
        ESP_LOGE(TAG, "Data is NULL");
        return false;
    }
    
    int len = strlen(data);
    int bytes_written = uart_write_bytes(UART_NUM, data, len);
    
    if (bytes_written != len) {
        ESP_LOGE(TAG, "Failed to send data. Written: %d, Expected: %d", bytes_written, len);
        return false;
    }
    
    ESP_LOGI(TAG, "Sent data: %s", data);
    return true;
}

bool uart_send_hello_world(void)
{
    const char* hello_msg = "Hello World from ESP32!\r\n";
    return uart_send_string(hello_msg);
}
