/**
 * @file tcp_server.h
 * @brief TCP服务器 - 开放端口给上位机
 */

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdint.h>
#include <stdbool.h>

// 协议常量
#define FRAME_HEADER 0xAA55
#define FRAME_TAIL 0x55AA
#define CMD_HEARTBEAT 0x0001
#define CMD_STATUS_QUERY 0x0002
#define CMD_STATUS_REPORT 0x0003
#define CMD_START_MEASURE 0x0200
#define CMD_STOP_MEASURE 0x0201
#define CMD_WIND_DATA 0x0301
#define CMD_ACK 0x0080

/**
 * @brief 初始化TCP服务器
 * @param port 监听端口
 * @return 0成功，-1失败
 */
int tcp_server_init(int port);

/**
 * @brief 启动TCP服务器
 */
void tcp_server_start(void);

/**
 * @brief 停止TCP服务器
 */
void tcp_server_stop(void);

/**
 * @brief 发送数据
 * @param data 数据
 * @param len 长度
 * @return 发送字节数
 */
int tcp_send(const uint8_t *data, int len);

/**
 * @brief 检查是否有客户端
 */
bool tcp_has_client(void);

#endif
