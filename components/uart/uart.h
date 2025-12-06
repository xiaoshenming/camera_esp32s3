#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// UART 组件头文件
// 包含串口相关的函数声明和数据结构

/**
 * @brief 初始化UART
 * @return true 成功，false 失败
 */
bool uart_init(void);

/**
 * @brief 发送字符串
 * @param data 要发送的字符串
 * @return true 成功，false 失败
 */
bool uart_send_string(const char* data);

/**
 * @brief 发送hello world消息
 * @return true 成功，false 失败
 */
bool uart_send_hello_world(void);

#ifdef __cplusplus
}
#endif

#endif // UART_H
