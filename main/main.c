#include "main.h"
#include "camera.h"
#include "uart.h"

void app_main(void)
{
    // 主程序入口
    // 初始化串口组件
    if (uart_init()) {
        // 串口初始化成功，发送hello world
        uart_send_hello_world();
    }
    
    // 初始化摄像头组件
    if (camera_init()) {
        // 摄像头初始化成功
        camera_capture();
    }
}
