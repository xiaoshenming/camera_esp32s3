#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>
#include "esp_err.h"

// WiFi配置 - 请修改为你的WiFi信息
#ifndef WIFI_SSID
#define WIFI_SSID "309Study"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "ai123321"
#endif

// WiFi事件处理函数类型
typedef void (*wifi_event_handler_t)(void);

/**
 * @brief 初始化WiFi
 * @return true 成功，false 失败
 */
bool wifi_init(void);

/**
 * @brief 连接到WiFi网络
 * @return true 成功，false 失败
 */
bool wifi_connect(void);

/**
 * @brief 获取设备IP地址
 * @return IP地址字符串，失败返回NULL
 */
char* wifi_get_ip_address(void);

/**
 * @brief 检查WiFi连接状态
 * @return true 已连接，false 未连接
 */
bool wifi_is_connected(void);

/**
 * @brief 等待WiFi连接
 * @param timeout_ms 超时时间（毫秒）
 * @return true 连接成功，false 超时
 */
bool wifi_wait_connection(int timeout_ms);

#endif // WIFI_H
