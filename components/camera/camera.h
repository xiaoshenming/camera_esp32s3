#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

// Camera 组件头文件
// 包含摄像头相关的函数声明和数据结构

/**
 * @brief 初始化摄像头
 * @return true 成功，false 失败
 */
bool camera_init(void);

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
 * @brief 启动摄像头RTSP推流
 * @return true 成功，false 失败
 */
bool camera_start_rtsp_stream(void);

/**
 * @brief 停止摄像头RTSP推流
 * @return true 成功，false 失败
 */
bool camera_stop_rtsp_stream(void);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_H
