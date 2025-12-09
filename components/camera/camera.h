#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Camera 组件头文件
// 包含摄像头相关的函数声明和数据结构

// 摄像头功能配置结构体
typedef struct {
    bool enable_lcd_display;    // 是否启用LCD显示
    bool enable_fps_monitor;    // 是否启用帧率监控
    bool enable_capture_task;    // 是否启用捕获任务
    uint32_t xclk_freq_hz;      // 摄像头时钟频率
    uint32_t frame_size;        // 帧尺寸
} camera_user_config_t;

/**
 * @brief 初始化摄像头
 * @return true 成功，false 失败
 */
bool camera_init(void);

/**
 * @brief 设置摄像头配置
 * @param config 配置参数
 * @return true 成功，false 失败
 */
bool camera_set_config(const camera_user_config_t *config);

/**
 * @brief 获取当前摄像头配置
 * @return 当前配置指针
 */
const camera_user_config_t* camera_get_config(void);

/**
 * @brief 启动摄像头功能（根据配置启动相应功能）
 * @return true 成功，false 失败
 */
bool camera_start(void);

/**
 * @brief 停止摄像头功能
 * @return true 成功，false 失败
 */
bool camera_stop(void);

/**
 * @brief 启动摄像头到LCD的实时显示
 * @return true 成功，false 失败
 */
bool camera_start_lcd_display(void);

/**
 * @brief 停止摄像头到LCD的显示
 * @return true 成功，false 失败
 */
bool camera_stop_lcd_display(void);

/**
 * @brief 启动帧率监控
 * @return true 成功，false 失败
 */
bool camera_start_fps_monitor(void);

/**
 * @brief 停止帧率监控
 * @return true 成功，false 失败
 */
bool camera_stop_fps_monitor(void);

/**
 * @brief 获取当前帧率
 * @param camera_fps 摄像头帧率输出
 * @param lcd_fps LCD帧率输出
 * @return true 成功，false 失败
 */
bool camera_get_fps(float *camera_fps, float *lcd_fps);

/**
 * @brief 捕获图像
 * @return true 成功，false 失败
 */
bool camera_capture(void);

/**
 * @brief 获取图像数据
 * @return 图像数据指针
 */
uint8_t* camera_get_image_data(void);

/**
 * @brief 启动FPV模式（WiFi UDP传输）
 * @return true 成功，false 失败
 */
bool camera_start_fpv_mode(void);

/**
 * @brief 停止FPV模式
 * @return true 成功，false 失败
 */
bool camera_stop_fpv_mode(void);

/**
 * @brief FPV传输任务
 * @param arg 任务参数
 */
static void camera_fpv_task(void *arg);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_H
