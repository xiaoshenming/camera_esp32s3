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
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_QVGA;  // 修改为320x240以匹配LCD分辨率
    config.fb_count = 2;  // 增加帧缓冲区数量以支持RTSP
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

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
    
    while (display_running || rtsp_running) {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame) {
            // 如果LCD显示运行，发送到LCD
            if (display_running) {
                // 创建帧的副本用于LCD显示
                camera_fb_t *lcd_frame = (camera_fb_t *)malloc(sizeof(camera_fb_t));
                if (lcd_frame) {
                    memcpy(lcd_frame, frame, sizeof(camera_fb_t));
                    // 不复制数据，直接使用原始数据（LCD函数会处理缩放）
                    lcd_draw_camera_frame(0, 0, frame->width, frame->height, frame->buf);
                    free(lcd_frame);
                }
            }
            
            // 如果RTSP推流运行，发送到RTSP
            if (rtsp_running) {
                // 发送原始帧到RTSP服务器
                rtsp_send_frame(frame);
            } else {
                // 如果RTSP没有运行，释放帧
                esp_camera_fb_return(frame);
            }
        } else {
            ESP_LOGW(TAG, "Failed to get camera frame");
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        // 控制帧率到10fps
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    ESP_LOGI(TAG, "Camera unified task stopped");
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

// RTSP推流任务
static void camera_rtsp_task(void *arg)
{
    ESP_LOGI(TAG, "Camera RTSP task started");
    
    while (rtsp_running) {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame) {
            // 发送帧到RTSP服务器
            rtsp_send_frame(frame);
        } else {
            ESP_LOGW(TAG, "Failed to get camera frame for RTSP");
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        // 控制帧率到10fps
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    ESP_LOGI(TAG, "Camera RTSP task stopped");
    vTaskDelete(NULL);
}

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
