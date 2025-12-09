#include "camera.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lcd.h"
#include "wifi.h"
#include "sensor.h"

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

#define DEFAULT_XCLK_FREQ_HZ 24000000  // 使用立创例程的24MHz时钟

// LCD显示队列句柄
static QueueHandle_t xQueueLCDFrame = NULL;
static TaskHandle_t camera_task_handle = NULL;
static TaskHandle_t lcd_task_handle = NULL;
static TaskHandle_t fps_monitor_task_handle = NULL;
static TaskHandle_t fpv_task_handle = NULL;
static bool camera_running = false;
static bool lcd_display_running = false;
static bool fps_monitor_running = false;
static bool fpv_running = false;

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
    .frame_size = FRAMESIZE_QQVGA  // 默认使用QQVGA
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
    config.pin_sccb_sda = -1;                // 使用已经初始化的I2C接口
    config.pin_sccb_scl = CAMERA_PIN_SIOC;   // 使用SCL引脚
    config.sccb_i2c_port = 0;                // 使用I2C端口0
    config.pin_pwdn = CAMERA_PIN_PWDN;
    config.pin_reset = CAMERA_PIN_RESET;
    config.xclk_freq_hz = current_config.xclk_freq_hz;
    config.pixel_format = PIXFORMAT_RGB565;  // 使用原始RGB565格式
    config.frame_size = current_config.frame_size;  // 使用配置的分辨率
    config.jpeg_quality = 12;
    config.fb_count = 2;                     // 使用双缓冲
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;  // 使用立创例程的grab模式

    // 摄像头初始化
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return false;
    }

    // 等待摄像头稳定
    vTaskDelay(pdMS_TO_TICKS(500));

    sensor_t *s = esp_camera_sensor_get(); // 获取摄像头型号
    if (s) {
        ESP_LOGI(TAG, "Camera sensor detected, PID: 0x%x", s->id.PID);
        
        // GC0308特殊处理 - 使用最简化配置
        if (s->id.PID == GC0308_PID) {
            ESP_LOGI(TAG, "Configuring GC0308 camera with minimal settings...");
            
            // 只设置镜像，其他所有参数都保持默认
            s->set_hmirror(s, 1);
            
            ESP_LOGI(TAG, "GC0308 camera configured with mirror only");
        } else {
            // 通用摄像头设置
            s->set_brightness(s, 0);     // 亮度
            s->set_contrast(s, 0);        // 对比度
            s->set_saturation(s, 0);      // 饱和度
        }
        
        // 设置分辨率 - 先尝试QQVGA，如果失败再尝试其他分辨率
        int ret = -1;
        framesize_t resolutions_to_try[] = {FRAMESIZE_QQVGA, FRAMESIZE_QCIF, FRAMESIZE_HQVGA, FRAMESIZE_QVGA};
        int num_resolutions = sizeof(resolutions_to_try) / sizeof(resolutions_to_try[0]);
        
        for (int i = 0; i < num_resolutions; i++) {
            ret = s->set_framesize(s, resolutions_to_try[i]);
            if (ret == 0) {
                current_config.frame_size = resolutions_to_try[i];
                ESP_LOGI(TAG, "Camera resolution set to: %dx%d", 
                         resolution[resolutions_to_try[i]].width,
                         resolution[resolutions_to_try[i]].height);
                break;
            } else {
                ESP_LOGW(TAG, "Failed to set resolution %dx%d, trying next...", 
                         resolution[resolutions_to_try[i]].width,
                         resolution[resolutions_to_try[i]].height);
            }
        }
        
        if (ret != 0) {
            ESP_LOGE(TAG, "Failed to set any resolution, camera may not work properly");
        }
    } else {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return false;
    }
    
    // 再次等待摄像头设置生效
    vTaskDelay(pdMS_TO_TICKS(500));
    
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
    
    // 控制帧率 - 目标30FPS，每33ms一帧
    const TickType_t frame_delay = pdMS_TO_TICKS(33);  // 30 FPS
    TickType_t last_frame_time = xTaskGetTickCount();
    
    while (camera_running) {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame) {
            camera_frame_count++;  // 统计摄像头捕获帧数
            
            // 如果启用了FPV模式，也发送到FPV
            if (fpv_running) {
                // 发送帧数据到FPV
                static uint16_t fpv_frame_id = 0;
                if (!wifi_send_camera_frame(frame->buf, frame->len, fpv_frame_id)) {
                    ESP_LOGW(TAG, "Failed to send FPV frame %d", fpv_frame_id);
                } else {
                    ESP_LOGD(TAG, "Sent FPV frame %d, size: %d", fpv_frame_id, frame->len);
                }
                fpv_frame_id++;
            }
            
            // 将帧发送到LCD显示队列
            if (!xQueueSend(xQueueLCDFrame, &frame, pdMS_TO_TICKS(10))) {
                // 如果队列满了，释放帧
                esp_camera_fb_return(frame);
            }
            
            // 控制帧率 - 等待到下一帧时间
            TickType_t current_time = xTaskGetTickCount();
            TickType_t elapsed = current_time - last_frame_time;
            
            if (elapsed < frame_delay) {
                vTaskDelay(frame_delay - elapsed);
            }
            
            last_frame_time = xTaskGetTickCount();
            
        } else {
            ESP_LOGW(TAG, "Failed to get camera frame");
            vTaskDelay(pdMS_TO_TICKS(50));  // 获取帧失败时的延迟
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
    
    // 如果不需要LCD显示但需要FPV，创建一个小的队列用于帧统计
    if (!current_config.enable_lcd_display && !xQueueLCDFrame) {
        xQueueLCDFrame = xQueueCreate(1, sizeof(camera_fb_t *));
        if (xQueueLCDFrame == NULL) {
            ESP_LOGE(TAG, "Failed to create FPV frame queue");
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


// 启动FPV模式（WiFi UDP传输）
bool camera_start_fpv_mode(void)
{
    if (fpv_running) {
        ESP_LOGW(TAG, "FPV mode already running");
        return true;
    }
    
    ESP_LOGI(TAG, "Starting FPV mode...");
    
    // 检查WiFi是否已连接
    if (!wifi_is_connected()) {
        ESP_LOGE(TAG, "WiFi not connected. Please ensure WiFi is initialized first.");
        return false;
    }
    
    // 初始化UDP广播
    if (!wifi_udp_broadcast_init(UDP_PORT)) {
        ESP_LOGE(TAG, "Failed to initialize UDP broadcast");
        return false;
    }
    
    // 获取并显示IP地址
    char* local_ip = wifi_get_local_ip();
    if (local_ip) {
        ESP_LOGI(TAG, "FPV server started on IP: %s, Port: %d", local_ip, UDP_PORT);
        free(local_ip);
    }
    
    fpv_running = true;
    
    ESP_LOGI(TAG, "FPV mode started successfully");
    return true;
}

// 停止FPV模式
bool camera_stop_fpv_mode(void)
{
    if (!fpv_running) {
        ESP_LOGW(TAG, "FPV mode not running");
        return true;
    }
    
    ESP_LOGI(TAG, "Stopping FPV mode...");
    
    fpv_running = false;
    
    // 等待任务结束
    if (fpv_task_handle) {
        vTaskDelete(fpv_task_handle);
        fpv_task_handle = NULL;
    }
    
    ESP_LOGI(TAG, "FPV mode stopped successfully");
    return true;
}
