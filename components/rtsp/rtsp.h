#ifndef RTSP_H
#define RTSP_H

#include <stdbool.h>
#include "esp_err.h"
#include "camera.h"

// RTSP服务器配置
#define RTSP_PORT 8554
#define RTSP_PATH "stream"
#define RTSP_MAX_CLIENTS 3

/**
 * @brief 初始化RTSP服务器
 * @return true 成功，false 失败
 */
bool rtsp_init(void);

/**
 * @brief 启动RTSP服务器
 * @return true 成功，false 失败
 */
bool rtsp_start(void);

/**
 * @brief 停止RTSP服务器
 * @return true 成功，false 失败
 */
bool rtsp_stop(void);

/**
 * @brief 检查RTSP服务器状态
 * @return true 运行中，false 已停止
 */
bool rtsp_is_running(void);

/**
 * @brief 获取RTSP流URL
 * @return RTSP URL字符串，失败返回NULL
 */
char* rtsp_get_stream_url(void);

/**
 * @brief 设置摄像头帧回调函数
 * @param fb 帧缓冲区指针
 */
void rtsp_send_frame(camera_fb_t* fb);

/**
 * @brief 获取连接的客户端数量
 * @return 客户端数量
 */
int rtsp_get_client_count(void);

#endif // RTSP_H
