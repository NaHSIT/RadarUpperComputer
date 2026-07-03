/**
 * @file tcp_server.c
 * @brief TCP服务器实现 - 开放端口给上位机
 */

#include "tcp_server.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

static int server_fd = -1;
static int client_fd = -1;
static pthread_t server_thread;
static volatile int running = 0;

// CRC16计算
static uint16_t calc_crc16(const uint8_t *data, int len) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= (data[i] << 8);
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
    }
    return crc;
}

// 服务器线程
static void* server_func(void* arg) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    uint8_t buffer[4096];

    printf("等待上位机连接...\n");

    while (running) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) continue;

        printf("上位机已连接: %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while (running) {
            int len = recv(client_fd, buffer, sizeof(buffer), 0);
            if (len <= 0) break;
        }

        printf("上位机断开\n");
        close(client_fd);
        client_fd = -1;
    }
    return NULL;
}

int tcp_server_init(int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return -1;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        return -1;
    }

    listen(server_fd, 5);
    printf("TCP服务器初始化完成，端口: %d\n", port);
    return 0;
}

void tcp_server_start(void) {
    running = 1;
    pthread_create(&server_thread, NULL, server_func, NULL);
}

void tcp_server_stop(void) {
    running = 0;
    if (client_fd >= 0) close(client_fd);
    if (server_fd >= 0) close(server_fd);
}

int tcp_send(const uint8_t *data, int len) {
    if (client_fd < 0) return -1;
    return send(client_fd, data, len, 0);
}

bool tcp_has_client(void) {
    return client_fd >= 0;
}
