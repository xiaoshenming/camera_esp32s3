#include "main.h"
#include "camera.h"
#include "uart.h"
#include "lcd.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    ESP_LOGI("main", "ESP32 Camera LCD Display Starting...");
    
    // 主程序入口
    // 初始化串口组件
    if (uart_init()) {
        // 串口初始化成功，发送hello world
        uart_send_hello_world();
    }
    
    // 初始化I2C接口（LCD和摄像头都需要）
    if (!lcd_i2c_init()) {
        ESP_LOGE("main", "I2C initialization failed");
        return;
    }
    
    // 初始化IO扩展芯片（必须在摄像头初始化之前，因为摄像头电源由PCA9557控制）
    if (!lcd_pca9557_init()) {
        ESP_LOGE("main", "PCA9557 initialization failed");
        return;
    }
    
    // 初始化摄像头组件（摄像头会使用已初始化的I2C总线）
    if (!camera_init()) {
        ESP_LOGE("main", "Camera initialization failed");
        return;
    }
    
    // 初始化LCD显示屏
    if (!lcd_init()) {
        ESP_LOGE("main", "LCD initialization failed");
        return;
    }
    
    // 启动摄像头到LCD的实时显示
    if (!camera_start_lcd_display()) {
        ESP_LOGE("main", "Failed to start camera LCD display");
        return;
    }
    
    ESP_LOGI("main", "Camera LCD display started successfully!");
    
    // 主循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI("main", "Camera LCD display is running...");
    }
}
