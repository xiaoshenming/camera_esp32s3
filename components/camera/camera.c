#include "camera.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lcd.h"

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

#define DEFAULT_XCLK_FREQ_HZ 40000000  // 默认40MHz以提高帧率

// LCD显示队列句柄
static QueueHandle_t xQueueLCDFrame = NULL;
static TaskHandle_t camera_task_handle = NULL;
static TaskHandle_t lcd_task_handle = NULL;
static TaskHandle_t fps_monitor_task_handle = NULL;
static bool camera_running = false;
static bool lcd_display_running = false;
static bool fps_monitor_running = false;

// 帧率统计变量
static uint32_t camera_frame_count = 0;
static uint32_t lcd_frame_count = 0;
static uint32_t last_fps_time = 0;
static float camera_fps = 0.0f;
static float lcd_fps = 0.0f;

// 当前摄像头配置
static camera_user_config_t current_config = {
    .enable_lcd_display = true,
    .enable_fps_monitor = true,
    .enable_capture_task = true,
    .xclk_freq_hz = DEFAULT_XCLK_FREQ_HZ,
    .frame_size = FRAMESIZE_QVGA
};

bool camera_init(void)
{
    ESP_LOGI(TAG, "Initializing camera...");
    
    // 打开摄像头电源
    lcd_dvp_pwdn(0);
    
    // 等待摄像头电源稳定
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 使用当前配置初始化摄像头
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
    config.xclk_freq_hz = current_config.xclk_freq_hz;
    config.pixel_format = PIXFORMAT_RGB565;  // 使用原始RGB565格式
    config.frame_size = current_config.frame_size;
    config.fb_count = 2;                     // 双缓冲提高性能
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

// 设置摄像头配置
bool camera_set_config(const camera_user_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Invalid config pointer");
        return false;
    }
    
    // 如果摄像头正在运行，不允许修改关键配置
    if (camera_running) {
        ESP_LOGW(TAG, "Cannot change config while camera is running");
        return false;
    }
    
    current_config = *config;
    ESP_LOGI(TAG, "Camera config updated");
    return true;
}

// 获取当前摄像头配置
const camera_user_config_t* camera_get_config(void)
{
    return &current_config;
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

// LCD处理任务
static void camera_lcd_task(void *arg)
{
    camera_fb_t *frame = NULL;
    
    ESP_LOGI(TAG, "LCD display task started");
    
    while (lcd_display_running) {
        if (xQueueReceive(xQueueLCDFrame, &frame, pdMS_TO_TICKS(100))) {
            if (frame) {
                // 显示摄像头帧到LCD
                lcd_draw_camera_frame(0, 0, frame->width, frame->height, frame->buf);
                lcd_frame_count++;  // 统计LCD显示帧数
                esp_camera_fb_return(frame);
            }
        }
    }
    
    ESP_LOGI(TAG, "LCD display task stopped");
    vTaskDelete(NULL);
}

// 帧率监控任务
static void fps_monitor_task(void *arg)
{
    ESP_LOGI(TAG, "FPS monitor task started");
    
    // 初始化时间戳
    last_fps_time = xTaskGetTickCount();
    
    while (fps_monitor_running) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // 每秒计算一次
        
        // 计算经过的时间（秒）
        uint32_t current_time = xTaskGetTickCount();
        float elapsed_time = (current_time - last_fps_time) / 1000.0f;
        
        // 计算FPS
        camera_fps = camera_frame_count / elapsed_time;
        lcd_fps = lcd_frame_count / elapsed_time;
        
        // 输出帧率信息
        ESP_LOGI(TAG, "Camera FPS: %.1f, LCD FPS: %.1f", camera_fps, lcd_fps);
        
        // 重置计数器
        camera_frame_count = 0;
        lcd_frame_count = 0;
        last_fps_time = current_time;
    }
    
    ESP_LOGI(TAG, "FPS monitor task stopped");
    vTaskDelete(NULL);
}

// 摄像头处理任务
static void camera_capture_task(void *arg)
{
    ESP_LOGI(TAG, "Camera capture task started");
    
    while (camera_running) {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame) {
            camera_frame_count++;  // 统计摄像头捕获帧数
            // 将帧发送到LCD显示队列
            if (!xQueueSend(xQueueLCDFrame, &frame, pdMS_TO_TICKS(10))) {
                // 如果队列满了，释放帧
                esp_camera_fb_return(frame);
            }
        } else {
            ESP_LOGW(TAG, "Failed to get camera frame");
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    
    ESP_LOGI(TAG, "Camera capture task stopped");
    vTaskDelete(NULL);
}

// 启动摄像头到LCD的实时显示
bool camera_start_lcd_display(void)
{
    if (lcd_display_running) {
        ESP_LOGW(TAG, "Camera LCD display already running");
        return true;
    }
    
    ESP_LOGI(TAG, "Starting camera LCD display...");
    
    // 创建LCD显示队列
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    if (xQueueLCDFrame == NULL) {
        ESP_LOGE(TAG, "Failed to create LCD frame queue");
        return false;
    }
    
    lcd_display_running = true;
    
    // 创建摄像头捕获任务
    BaseType_t ret = xTaskCreatePinnedToCore(
        camera_capture_task, 
        "camera_capture", 
        3 * 1024, 
        NULL, 
        5, 
        &camera_task_handle, 
        1
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create camera capture task");
        lcd_display_running = false;
        vQueueDelete(xQueueLCDFrame);
        xQueueLCDFrame = NULL;
        return false;
    }
    
    // 创建LCD显示任务
    ret = xTaskCreatePinnedToCore(
        camera_lcd_task, 
        "camera_lcd", 
        4 * 1024, 
        NULL, 
        5, 
        &lcd_task_handle, 
        0
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LCD display task");
        lcd_display_running = false;
        vTaskDelete(camera_task_handle);
        camera_task_handle = NULL;
        vQueueDelete(xQueueLCDFrame);
        xQueueLCDFrame = NULL;
        return false;
    }
    
    // 创建帧率监控任务
    ret = xTaskCreatePinnedToCore(
        fps_monitor_task, 
        "fps_monitor", 
        4 * 1024,  // 增加栈大小防止溢出
        NULL, 
        4, 
        &fps_monitor_task_handle, 
        1
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create FPS monitor task");
        lcd_display_running = false;
        vTaskDelete(camera_task_handle);
        camera_task_handle = NULL;
        vTaskDelete(lcd_task_handle);
        lcd_task_handle = NULL;
        vQueueDelete(xQueueLCDFrame);
        xQueueLCDFrame = NULL;
        return false;
    }
    
    ESP_LOGI(TAG, "Camera LCD display started successfully");
    return true;
}

// 启动摄像头功能（根据配置启动相应功能）
bool camera_start(void)
{
    ESP_LOGI(TAG, "Starting camera with config: LCD=%d, FPS=%d, Capture=%d", 
               current_config.enable_lcd_display, 
               current_config.enable_fps_monitor,
               current_config.enable_capture_task);
    
    camera_running = true;
    
    // 创建LCD显示队列（如果需要LCD显示）
    if (current_config.enable_lcd_display && !xQueueLCDFrame) {
        xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
        if (xQueueLCDFrame == NULL) {
            ESP_LOGE(TAG, "Failed to create LCD frame queue");
            camera_running = false;
            return false;
        }
    }
    
    // 创建摄像头捕获任务
    if (current_config.enable_capture_task) {
        BaseType_t ret = xTaskCreatePinnedToCore(
            camera_capture_task, 
            "camera_capture", 
            3 * 1024, 
            NULL, 
            5, 
            &camera_task_handle, 
            1
        );
        
        if (ret != pdPASS) {
            ESP_LOGE(TAG, "Failed to create camera capture task");
            camera_running = false;
            if (xQueueLCDFrame) {
                vQueueDelete(xQueueLCDFrame);
                xQueueLCDFrame = NULL;
            }
            return false;
        }
    }
    
    // 创建LCD显示任务
    if (current_config.enable_lcd_display) {
        lcd_display_running = true;
        BaseType_t ret = xTaskCreatePinnedToCore(
            camera_lcd_task, 
            "camera_lcd", 
            4 * 1024, 
            NULL, 
            5, 
            &lcd_task_handle, 
            0
        );
        
        if (ret != pdPASS) {
            ESP_LOGE(TAG, "Failed to create LCD display task");
            camera_running = false;
            if (camera_task_handle) {
                vTaskDelete(camera_task_handle);
                camera_task_handle = NULL;
            }
            if (xQueueLCDFrame) {
                vQueueDelete(xQueueLCDFrame);
                xQueueLCDFrame = NULL;
            }
            return false;
        }
    }
    
    // 创建帧率监控任务
    if (current_config.enable_fps_monitor) {
        fps_monitor_running = true;
        BaseType_t ret = xTaskCreatePinnedToCore(
            fps_monitor_task, 
            "fps_monitor", 
            4 * 1024, 
            NULL, 
            4, 
            &fps_monitor_task_handle, 
            1
        );
        
        if (ret != pdPASS) {
            ESP_LOGE(TAG, "Failed to create FPS monitor task");
            camera_running = false;
            if (camera_task_handle) {
                vTaskDelete(camera_task_handle);
                camera_task_handle = NULL;
            }
            if (lcd_task_handle) {
                vTaskDelete(lcd_task_handle);
                lcd_task_handle = NULL;
            }
            if (xQueueLCDFrame) {
                vQueueDelete(xQueueLCDFrame);
                xQueueLCDFrame = NULL;
            }
            return false;
        }
    }
    
    ESP_LOGI(TAG, "Camera started successfully");
    return true;
}

// 停止摄像头功能
bool camera_stop(void)
{
    ESP_LOGI(TAG, "Stopping camera...");
    
    camera_running = false;
    lcd_display_running = false;
    fps_monitor_running = false;
    
    // 等待任务结束
    if (camera_task_handle) {
        vTaskDelete(camera_task_handle);
        camera_task_handle = NULL;
    }
    
    if (lcd_task_handle) {
        vTaskDelete(lcd_task_handle);
        lcd_task_handle = NULL;
    }
    
    if (fps_monitor_task_handle) {
        vTaskDelete(fps_monitor_task_handle);
        fps_monitor_task_handle = NULL;
    }
    
    // 清理队列
    if (xQueueLCDFrame) {
        vQueueDelete(xQueueLCDFrame);
        xQueueLCDFrame = NULL;
    }
    
    ESP_LOGI(TAG, "Camera stopped successfully");
    return true;
}

// 启动帧率监控
bool camera_start_fps_monitor(void)
{
    if (fps_monitor_running) {
        ESP_LOGW(TAG, "FPS monitor already running");
        return true;
    }
    
    fps_monitor_running = true;
    BaseType_t ret = xTaskCreatePinnedToCore(
        fps_monitor_task, 
        "fps_monitor", 
        4 * 1024, 
        NULL, 
        4, 
        &fps_monitor_task_handle, 
        1
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create FPS monitor task");
        fps_monitor_running = false;
        return false;
    }
    
    ESP_LOGI(TAG, "FPS monitor started successfully");
    return true;
}

// 停止帧率监控
bool camera_stop_fps_monitor(void)
{
    if (!fps_monitor_running) {
        ESP_LOGW(TAG, "FPS monitor not running");
        return true;
    }
    
    fps_monitor_running = false;
    
    if (fps_monitor_task_handle) {
        vTaskDelete(fps_monitor_task_handle);
        fps_monitor_task_handle = NULL;
    }
    
    ESP_LOGI(TAG, "FPS monitor stopped successfully");
    return true;
}

// 获取当前帧率
bool camera_get_fps(float *camera_fps_out, float *lcd_fps_out)
{
    if (!camera_fps_out || !lcd_fps_out) {
        ESP_LOGE(TAG, "Invalid FPS output pointers");
        return false;
    }
    
    *camera_fps_out = camera_fps;
    *lcd_fps_out = lcd_fps;
    return true;
}

// 停止摄像头到LCD的显示
bool camera_stop_lcd_display(void)
{
    if (!lcd_display_running) {
        ESP_LOGW(TAG, "Camera LCD display not running");
        return true;
    }
    
    ESP_LOGI(TAG, "Stopping camera LCD display...");
    
    lcd_display_running = false;
    
    // 等待任务结束
    if (camera_task_handle) {
        vTaskDelete(camera_task_handle);
        camera_task_handle = NULL;
    }
    
    if (lcd_task_handle) {
        vTaskDelete(lcd_task_handle);
        lcd_task_handle = NULL;
    }
    
    if (fps_monitor_task_handle) {
        vTaskDelete(fps_monitor_task_handle);
        fps_monitor_task_handle = NULL;
    }
    
    // 清理队列
    if (xQueueLCDFrame) {
        vQueueDelete(xQueueLCDFrame);
        xQueueLCDFrame = NULL;
    }
    
    ESP_LOGI(TAG, "Camera LCD display stopped successfully");
    return true;
}
