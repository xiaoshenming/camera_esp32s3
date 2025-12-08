#include "camera.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lcd.h"
#include "rtsp.h"

static const char *TAG = "camera";

// 摄像头引脚定义
#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 5
#define CAMERA_PIN_SIOD 1
#define CAMERA_PIN_SIOC 2

#define CAMERA_PIN_D7 9
#define CAMERA_PIN_D6 4
#define CAMERA_PIN_D5 6
#define CAMERA_PIN_D4 15
#define CAMERA_PIN_D3 17
#define CAMERA_PIN_D2 8
#define CAMERA_PIN_D1 18
#define CAMERA_PIN_D0 16
#define CAMERA_PIN_VSYNC 3
#define CAMERA_PIN_HREF 46
#define CAMERA_PIN_PCLK 7

#define XCLK_FREQ_HZ 24000000

// LCD显示队列句柄
static QueueHandle_t xQueueLCDFrame = NULL;
static TaskHandle_t camera_task_handle = NULL;
static TaskHandle_t lcd_task_handle = NULL;
static TaskHandle_t rtsp_task_handle = NULL;
static bool display_running = false;
static bool rtsp_running = false;

bool camera_init(void)
{
    ESP_LOGI(TAG, "Initializing camera...");
    
    // 打开摄像头电源
    lcd_dvp_pwdn(0);
    
    // 等待摄像头电源稳定
    vTaskDelay(pdMS_TO_TICKS(100));
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_1;
    config.ledc_timer = LEDC_TIMER_1;
    config.pin_d0 = CAMERA_PIN_D0;
    config.pin_d1 = CAMERA_PIN_D1;
    config.pin_d2 = CAMERA_PIN_D2;
    config.pin_d3 = CAMERA_PIN_D3;
    config.pin_d4 = CAMERA_PIN_D4;
    config.pin_d5 = CAMERA_PIN_D5;
    config.pin_d6 = CAMERA_PIN_D6;
    config.pin_d7 = CAMERA_PIN_D7;
    config.pin_xclk = CAMERA_PIN_XCLK;
    config.pin_pclk = CAMERA_PIN_PCLK;
    config.pin_vsync = CAMERA_PIN_VSYNC;
    config.pin_href = CAMERA_PIN_HREF;
    config.pin_sccb_sda = -1;                // 使用已初始化的I2C接口
    config.pin_sccb_scl = CAMERA_PIN_SIOC;   // 使用SCL引脚
    config.sccb_i2c_port = 0;                // 使用I2C端口0
    config.pin_pwdn = CAMERA_PIN_PWDN;
    config.pin_reset = CAMERA_PIN_RESET;
    config.xclk_freq_hz = 12000000; // 进一步降低时钟频率到12MHz
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_VGA;  // 640x480以匹配RTSP期望
    config.fb_count = 4;  // 增加更多帧缓冲区数量
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;  // 使用最新帧模式，避免堆积

    // 摄像头初始化
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get(); // 获取摄像头型号
    if (s->id.PID == GC0308_PID) {
        s->set_hmirror(s, 1);  // 摄像头镜像
    }
    
    ESP_LOGI(TAG, "Camera initialized successfully");
    return true;
}

bool camera_capture(void)
{
    ESP_LOGI(TAG, "Capturing image...");
    // TODO: 实现图像捕获逻辑
    return true;
}

uint8_t* camera_get_image_data(void)
{
    ESP_LOGI(TAG, "Getting image data...");
    // TODO: 实现获取图像数据逻辑
    return NULL;
}

// 统一的摄像头处理任务（同时处理LCD显示和RTSP推流）
static void camera_unified_task(void *arg)
{
    ESP_LOGI(TAG, "Camera unified task started");

    // 帧获取统计
    uint32_t frame_count = 0;
    uint32_t error_count = 0;
    TickType_t last_frame_time = xTaskGetTickCount();
    const TickType_t frame_interval = pdMS_TO_TICKS(200); // 5fps = 200ms间隔

    while (display_running || rtsp_running) {
        // 使用更精确的帧率控制
        TickType_t current_time = xTaskGetTickCount();
        if ((current_time - last_frame_time) >= frame_interval) {
            camera_fb_t *frame = esp_camera_fb_get();
            if (frame) {
                frame_count++;
                error_count = 0; // 重置错误计数

                // 如果LCD显示运行，发送到LCD
                if (display_running) {
                    lcd_draw_camera_frame(0, 0, frame->width, frame->height, frame->buf);
                }

                // 如果RTSP推流运行，发送到RTSP
                if (rtsp_running) {
                    // 发送帧到RTSP服务器（注意：rtsp_send_frame会负责释放帧）
                    rtsp_send_frame(frame);
                } else {
                    // 如果RTSP没有运行，立即释放帧
                    esp_camera_fb_return(frame);
                }

                last_frame_time = current_time;

                // 每100帧记录一次统计信息
                if (frame_count % 100 == 0) {
                    ESP_LOGI(TAG, "Camera task: processed %u frames, current FPS: %.1f",
                             frame_count, 1000.0 / (frame_interval * portTICK_PERIOD_MS));
                }
            } else {
                error_count++;
                ESP_LOGW(TAG, "Failed to get camera frame (error #%u)", error_count);

                // 连续错误过多时进行更长的延迟
                if (error_count > 10) {
                    ESP_LOGE(TAG, "Too many consecutive errors, increasing delay");
                    vTaskDelay(pdMS_TO_TICKS(100));
                } else {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        } else {
            // 等待到下一帧时间
            vTaskDelay(1);
        }
    }

    ESP_LOGI(TAG, "Camera unified task stopped. Total frames: %u", frame_count);
    vTaskDelete(NULL);
}

// 启动摄像头到LCD的实时显示
bool camera_start_lcd_display(void)
{
    if (display_running) {
        ESP_LOGW(TAG, "Camera LCD display already running");
        return true;
    }
    
    ESP_LOGI(TAG, "Starting camera LCD display...");
    
    display_running = true;
    
    // 创建统一的摄像头任务
    BaseType_t ret = xTaskCreatePinnedToCore(
        camera_unified_task, 
        "camera_unified", 
        8 * 1024, 
        NULL, 
        5, 
        &camera_task_handle, 
        1
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create camera unified task");
        display_running = false;
        return false;
    }
    
    ESP_LOGI(TAG, "Camera LCD display started successfully");
    return true;
}

// 停止摄像头到LCD的显示
bool camera_stop_lcd_display(void)
{
    if (!display_running) {
        ESP_LOGW(TAG, "Camera LCD display not running");
        return true;
    }
    
    ESP_LOGI(TAG, "Stopping camera LCD display...");
    
    display_running = false;
    
    // 等待任务结束
    if (camera_task_handle) {
        vTaskDelete(camera_task_handle);
        camera_task_handle = NULL;
    }
    
    if (lcd_task_handle) {
        vTaskDelete(lcd_task_handle);
        lcd_task_handle = NULL;
    }
    
    // 清理队列
    if (xQueueLCDFrame) {
        vQueueDelete(xQueueLCDFrame);
        xQueueLCDFrame = NULL;
    }
    
    ESP_LOGI(TAG, "Camera LCD display stopped successfully");
    return true;
}

// RTSP推流任务 (已弃用，统一到camera_unified_task)
// static void camera_rtsp_task(void *arg)  // 保留注释以说明统一任务的实现

// 启动摄像头RTSP推流
bool camera_start_rtsp_stream(void)
{
    if (rtsp_running) {
        ESP_LOGW(TAG, "Camera RTSP stream already running");
        return true;
    }
    
    ESP_LOGI(TAG, "Starting camera RTSP stream...");
    
    rtsp_running = true;
    
    // 如果摄像头任务还没有运行，创建统一的摄像头任务
    if (!display_running) {
        BaseType_t ret = xTaskCreatePinnedToCore(
            camera_unified_task, 
            "camera_unified", 
            8 * 1024, 
            NULL, 
            6,  // 更高优先级
            &camera_task_handle, 
            1   // 在核心1运行
        );
        
        if (ret != pdPASS) {
            ESP_LOGE(TAG, "Failed to create camera unified task");
            rtsp_running = false;
            return false;
        }
    }
    
    ESP_LOGI(TAG, "Camera RTSP stream started successfully");
    return true;
}

// 停止摄像头RTSP推流
bool camera_stop_rtsp_stream(void)
{
    if (!rtsp_running) {
        ESP_LOGW(TAG, "Camera RTSP stream not running");
        return true;
    }
    
    ESP_LOGI(TAG, "Stopping camera RTSP stream...");
    
    rtsp_running = false;
    
    // 等待任务结束
    if (rtsp_task_handle) {
        vTaskDelete(rtsp_task_handle);
        rtsp_task_handle = NULL;
    }
    
    ESP_LOGI(TAG, "Camera RTSP stream stopped successfully");
    return true;
}
