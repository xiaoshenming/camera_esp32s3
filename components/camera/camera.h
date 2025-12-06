#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include <stdint.h>

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

#ifdef __cplusplus
}
#endif

#endif // CAMERA_H
