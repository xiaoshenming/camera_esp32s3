#ifndef LCD_H
#define LCD_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

// LCD 组件头文件
// 包含LCD显示相关的函数声明和数据结构

#define BSP_LCD_H_RES              (320)
#define BSP_LCD_V_RES              (240)

/**
 * @brief 初始化I2C接口
 * @return true 成功，false 失败
 */
bool lcd_i2c_init(void);

/**
 * @brief 初始化IO扩展芯片PCA9557
 * @return true 成功，false 失败
 */
bool lcd_pca9557_init(void);

/**
 * @brief 初始化LCD显示屏
 * @return true 成功，false 失败
 */
bool lcd_init(void);

/**
 * @brief 设置LCD颜色
 * @param color 颜色值 (RGB565格式)
 */
void lcd_set_color(uint16_t color);

/**
 * @brief 显示图片
 * @param x_start 起始X坐标
 * @param y_start 起始Y坐标
 * @param x_end 结束X坐标
 * @param y_end 结束Y坐标
 * @param gImage 图片数据指针
 */
void lcd_draw_picture(int x_start, int y_start, int x_end, int y_end, const unsigned char *gImage);

/**
 * @brief 显示摄像头帧
 * @param x_start 起始X坐标
 * @param y_start 起始Y坐标
 * @param width 宽度
 * @param height 高度
 * @param frame_buf 帧缓冲区指针
 */
void lcd_draw_camera_frame(int x_start, int y_start, int width, int height, const uint8_t *frame_buf);

/**
 * @brief 初始化LCD背光
 * @return true 成功，false 失败
 */
bool lcd_backlight_init(void);

/**
 * @brief 设置LCD背光亮度
 * @param brightness_percent 亮度百分比 (0-100)
 * @return true 成功，false 失败
 */
bool lcd_backlight_set(int brightness_percent);

/**
 * @brief 打开LCD背光
 * @return true 成功，false 失败
 */
bool lcd_backlight_on(void);

/**
 * @brief 关闭LCD背光
 * @return true 成功，false 失败
 */
bool lcd_backlight_off(void);

/**
 * @brief 控制摄像头电源引脚
 * @param level 0=打开摄像头，1=关闭摄像头
 */
void lcd_dvp_pwdn(uint8_t level);

#ifdef __cplusplus
}
#endif

#endif // LCD_H
