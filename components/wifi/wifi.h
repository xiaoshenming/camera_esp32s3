#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

// WiFi配置结构体
typedef struct {
    char ssid[32];
    char password[64];
} wifi_credentials_t;

// UDP数据包头部结构体
typedef struct __attribute__((packed)) {
    uint16_t magic;         // 魔数 0xFPFV
    uint16_t frame_id;      // 帧ID
    uint16_t packet_id;     // 包ID
    uint16_t total_packets; // 总包数
} udp_packet_header_t;

#define UDP_MAGIC_NUMBER 0x5056
#define UDP_PORT 8888
#define MAX_UDP_PAYLOAD_SIZE 1024
#define PACKETS_PER_FRAME ((320 * 240 * 2 + MAX_UDP_PAYLOAD_SIZE - 1) / MAX_UDP_PAYLOAD_SIZE)

/**
 * @brief 初始化WiFi STA模式
 * @param ssid WiFi名称
 * @param password WiFi密码
 * @return true 成功，false 失败
 */
bool wifi_init_sta(const char* ssid, const char* password);

/**
 * @brief 初始化UDP广播
 * @param port UDP端口
 * @return true 成功，false 失败
 */
bool wifi_udp_broadcast_init(uint16_t port);

/**
 * @brief 发送UDP数据包
 * @param data 数据指针
 * @param len 数据长度
 * @return 发送的字节数，-1表示失败
 */
int wifi_udp_send(const void* data, size_t len);

/**
 * @brief 获取WiFi连接状态
 * @return true 已连接，false 未连接
 */
bool wifi_is_connected(void);

/**
 * @brief 获取本地IP地址
 * @return IP地址字符串，需要调用者释放
 */
char* wifi_get_local_ip(void);

/**
 * @brief 发送摄像头帧数据
 * @param frame_data 帧数据指针
 * @param frame_size 帧大小
 * @param frame_id 帧ID
 * @return true 成功，false 失败
 */
bool wifi_send_camera_frame(const uint8_t* frame_data, size_t frame_size, uint16_t frame_id);

// WiFi信息结构体
typedef struct {
    char ssid[32];
    uint32_t ip;
    uint8_t channel;
} wifi_info_t;

/**
 * @brief 获取WiFi信息
 * @param info WiFi信息输出结构体
 * @return true 成功，false 失败
 */
bool wifi_get_info(wifi_info_t* info);

/**
 * @brief 获取WiFi统计信息
 * @param frames_sent 发送帧数
 * @param packets_sent 发送包数
 * @param bytes_sent 发送字节数
 * @param fps 帧率
 * @return true 成功，false 失败
 */
bool wifi_get_stats(uint32_t* frames_sent, uint32_t* packets_sent, uint32_t* bytes_sent, float* fps);

#ifdef __cplusplus
}
#endif

#endif // WIFI_H
