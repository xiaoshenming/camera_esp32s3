#include "main.h"
#include "camera.h"
#include "uart.h"
#include "lcd.h"
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
    
    // 配置摄像头功能模块
    // 示例1: 全功能模式（用于调试和开发）
    camera_user_config_t full_config = {
        .enable_lcd_display = true,      // 启用LCD显示
        .enable_fps_monitor = true,      // 启用帧率监控
        .enable_capture_task = true,      // 启用捕获任务
        .xclk_freq_hz = 40000000,       // 40MHz时钟
        .frame_size = FRAMESIZE_QVGA      // 320x240分辨率
    };
    
    // 示例2: 推流模式（用于实时图传，关闭LCD显示以提高性能）
    // camera_user_config_t streaming_config = {
    //     .enable_lcd_display = false,     // 关闭LCD显示节省CPU
    //     .enable_fps_monitor = true,      // 保留帧率监控
    //     .enable_capture_task = true,      // 启用捕获任务
    //     .xclk_freq_hz = 50000000,       // 50MHz时钟提升性能
    //     .frame_size = FRAMESIZE_QVGA      // 320x240分辨率
    // };
    
    // 示例3: 纯捕获模式（最高性能，只捕获不显示）
    // camera_user_config_t capture_only_config = {
    //     .enable_lcd_display = false,     // 关闭LCD显示
    //     .enable_fps_monitor = false,     // 关闭帧率监控
    //     .enable_capture_task = true,      // 启用捕获任务
    //     .xclk_freq_hz = 50000000,       // 50MHz最高性能
    //     .frame_size = FRAMESIZE_QVGA      // 320x240分辨率
    // };
    
    // 选择配置模式（这里使用全功能模式作为示例）
    camera_user_config_t selected_config = full_config;
    
    // 设置摄像头配置
    if (!camera_set_config(&selected_config)) {
        ESP_LOGE("main", "Failed to set camera config");
        return;
    }
    
    // 启动摄像头功能（根据配置自动启动相应模块）
    if (!camera_start()) {
        ESP_LOGE("main", "Failed to start camera");
        return;
    }
    
    ESP_LOGI("main", "Camera system started successfully!");
    ESP_LOGI("main", "Current config: LCD=%d, FPS=%d, Capture=%d, Clock=%lu", 
               selected_config.enable_lcd_display,
               selected_config.enable_fps_monitor, 
               selected_config.enable_capture_task,
               selected_config.xclk_freq_hz);
    
    // 主循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));  // 每5秒输出一次状态
        
        // 获取当前帧率
        float cam_fps, lcd_fps;
        if (camera_get_fps(&cam_fps, &lcd_fps)) {
            ESP_LOGI("main", "Status - Camera FPS: %.1f, LCD FPS: %.1f", cam_fps, lcd_fps);
        }
        
        // 这里可以添加配置切换逻辑
        // 例如：通过串口命令切换到推流模式
        // camera_set_config(&streaming_config);
        // camera_stop();
        // camera_start();
    }
}
