#include "main.h"
#include "camera.h"
#include "uart.h"
#include "lcd.h"
#include "wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"

void app_main(void)
{
    ESP_LOGI("main", "ESP32 Camera System Starting...");
    
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
    
    // 配置摄像头功能模块（在初始化之前设置配置）
    // FPV模式配置（使用立创例程的稳定配置）
    camera_user_config_t fpv_config = {
        .enable_lcd_display = false,     // 关闭LCD显示节省CPU
        .enable_fps_monitor = true,      // 保留帧率监控
        .enable_capture_task = true,     // 启用捕获任务为FPV提供数据
        .xclk_freq_hz = 24000000,       // 24MHz时钟（立创例程验证稳定）
        .frame_size = FRAMESIZE_QQVGA    // 160x120分辨率（实际工作分辨率）
    };
    
    // 选择FPV模式配置
    camera_user_config_t selected_config = fpv_config;
    
    // 设置摄像头配置（在初始化之前）
    if (!camera_set_config(&selected_config)) {
        ESP_LOGE("main", "Failed to set camera config");
        return;
    }
    
    // 初始化摄像头组件（摄像头会使用已初始化的I2C总线和配置）
    if (!camera_init()) {
        ESP_LOGE("main", "Camera initialization failed");
        return;
    }
    
    // 初始化WiFi组件（用于FPV图传）
    if (!wifi_init_sta(WIFI_SSID, WIFI_PASSWORD)) {
        ESP_LOGE("main", "WiFi initialization failed");
        return;
    }
    
    // 等待WiFi连接
    ESP_LOGI("main", "Waiting for WiFi connection...");
    int retry_count = 0;
    while (!wifi_is_connected() && retry_count < 20) {
        vTaskDelay(pdMS_TO_TICKS(500));
        retry_count++;
        ESP_LOGI("main", "Waiting for WiFi connection... %d/20", retry_count);
    }
    
    if (!wifi_is_connected()) {
        ESP_LOGE("main", "WiFi connection failed");
        return;
    }
    ESP_LOGI("main", "WiFi connected successfully!");
    
    // 启动摄像头功能（根据配置自动启动相应模块）
    if (!camera_start()) {
        ESP_LOGE("main", "Failed to start camera");
        return;
    }
    
    // 启动FPV模式（低延迟图传）
    ESP_LOGI("main", "Starting FPV mode...");
    if (!camera_start_fpv_mode()) {
        ESP_LOGE("main", "Failed to start FPV mode");
        return;
    }
    
    ESP_LOGI("main", "FPV Camera system started successfully!");
    ESP_LOGI("main", "Current config: LCD=%d, FPS=%d, Capture=%d, Clock=%lu", 
               selected_config.enable_lcd_display,
               selected_config.enable_fps_monitor, 
               selected_config.enable_capture_task,
               selected_config.xclk_freq_hz);
    
    // 获取WiFi信息用于显示
    wifi_info_t wifi_info;
    if (wifi_get_info(&wifi_info)) {
        // 将uint32_t IP地址转换为点分十进制格式
        uint8_t* ip_bytes = (uint8_t*)&wifi_info.ip;
        ESP_LOGI("main", "WiFi Info - SSID: %s, IP: %d.%d.%d.%d, Channel: %d", 
                  wifi_info.ssid, ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3], wifi_info.channel);
    }
    
    // 主循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));  // 每5秒输出一次状态
        
        // 获取FPV传输统计信息
        uint32_t frames_sent, packets_sent, bytes_sent;
        float fps;
        if (wifi_get_stats(&frames_sent, &packets_sent, &bytes_sent, &fps)) {
            ESP_LOGI("main", "FPV Status - FPS: %.1f, Frames: %lu, Packets: %lu, Throughput: %.2f Mbps", 
                       fps, frames_sent, packets_sent, (float)bytes_sent * 8 / 5000 / 1000000);
        }
        
        // 获取摄像头帧率（如果启用了监控）
        if (selected_config.enable_fps_monitor) {
            float cam_fps, lcd_fps;
            if (camera_get_fps(&cam_fps, &lcd_fps)) {
                ESP_LOGI("main", "Camera Status - Camera FPS: %.1f", cam_fps);
            }
        }
    }
}
