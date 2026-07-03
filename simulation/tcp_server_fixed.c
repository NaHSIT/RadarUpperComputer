/**
 * @file tcp_server.c
 * @brief TCP服务器 - 完整命令处理
 */

#include "tcp_server.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>

static int server_fd = -1;
static int client_fd = -1;
static pthread_t server_thread;
static volatile int running = 0;
static volatile int measuring = 0;

uint16_t calc_crc16(const uint8_t *data, int len) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= (data[i] << 8);
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
    }
    return crc;
}

uint8_t* build_frame(int cmd, int seq, const uint8_t *payload, int payload_len, int *out_len) {
    int frame_len = 14 + payload_len;
    uint8_t *frame = (uint8_t*)malloc(frame_len);
    if (!frame) return NULL;
    int offset = 0;
    frame[offset++] = (FRAME_HEADER >> 8) & 0xFF;
    frame[offset++] = FRAME_HEADER & 0xFF;
    int length = 2 + 4 + payload_len + 2;
    frame[offset++] = (length >> 8) & 0xFF;
    frame[offset++] = length & 0xFF;
    frame[offset++] = (cmd >> 8) & 0xFF;
    frame[offset++] = cmd & 0xFF;
    frame[offset++] = (seq >> 24) & 0xFF;
    frame[offset++] = (seq >> 16) & 0xFF;
    frame[offset++] = (seq >> 8) & 0xFF;
    frame[offset++] = seq & 0xFF;
    if (payload && payload_len > 0) {
        memcpy(frame + offset, payload, payload_len);
        offset += payload_len;
    }
    uint16_t crc = calc_crc16(frame + 2, offset - 2);
    frame[offset++] = (crc >> 8) & 0xFF;
    frame[offset++] = crc & 0xFF;
    frame[offset++] = (FRAME_TAIL >> 8) & 0xFF;
    frame[offset++] = FRAME_TAIL & 0xFF;
    *out_len = frame_len;
    return frame;
}

int encode_wind_field_data(uint8_t *payload, int *payload_len) {
    int offset = 0;
    uint64_t ts = 0;
    memcpy(payload + offset, &ts, 8); offset += 8;
    payload[offset++] = 0;
    payload[offset++] = (30 >> 8) & 0xFF;
    payload[offset++] = 30 & 0xFF;
    float res = 10.0f;
    memcpy(payload + offset, &res, 4); offset += 4;
    float max_range = 300.0f;
    memcpy(payload + offset, &max_range, 4); offset += 4;
    payload[offset++] = 0; payload[offset++] = 0; payload[offset++] = 0;
    for (int i = 0; i < 30; i++) {
        float speed = 5.0f + i * 0.3f;
        memcpy(payload + offset, &speed, 4); offset += 4;
    }
    for (int i = 0; i < 30; i++) {
        float dir = 180.0f + (i - 15) * 0.5f;
        memcpy(payload + offset, &dir, 4); offset += 4;
    }
    for (int i = 0; i < 30; i++) {
        float vspeed = 0.0f;
        memcpy(payload + offset, &vspeed, 4); offset += 4;
    }
    for (int i = 0; i < 30; i++) {
        int conf = 95 - i;
        payload[offset++] = conf > 100 ? 100 : (conf < 0 ? 0 : conf);
    }
    float snr = 25.0f;
    memcpy(payload + offset, &snr, 4); offset += 4;
    float turb = 0.1f;
    memcpy(payload + offset, &turb, 4); offset += 4;
    *payload_len = offset;
    return 0;
}

static void* server_func(void* arg) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    uint8_t buffer[4096];
    printf("等待上位机连接...\n");
    while (running) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) continue;
        printf("上位机已连接: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        while (running) {
            int len = recv(client_fd, buffer, sizeof(buffer), 0);
            if (len <= 0) break;
            if (len >= 14 && buffer[0] == 0xAA && buffer[1] == 0x55) {
                int cmd = (buffer[4] << 8) | buffer[5];
                int seq = (buffer[6] << 24) | (buffer[7] << 16) | (buffer[8] << 8) | buffer[9];
                printf("收到命令: 0x%04X\n", cmd);
                uint8_t *resp = NULL;
                int resp_len = 0;
                if (cmd == 0x0001) {
                    resp = build_frame(0x0080, seq, NULL, 0, &resp_len);
                } else if (cmd == 0x0002) {
                    resp = build_frame(0x0003, seq, NULL, 0, &resp_len);
                } else if (cmd == 0x0006) {
                    resp = build_frame(0x0007, seq, (uint8_t*)"1.0.0", 6, &resp_len);
                } else if (cmd == 0x0200) {
                    measuring = 1;
                    resp = build_frame(0x0080, seq, NULL, 0, &resp_len);
                } else if (cmd == 0x0201) {
                    measuring = 0;
                    resp = build_frame(0x0080, seq, NULL, 0, &resp_len);
                } else if (cmd == 0x0202) {
                    resp = build_frame(0x0080, seq, NULL, 0, &resp_len);
                } else {
                    resp = build_frame(0x0080, seq, NULL, 0, &resp_len);
                }
                if (resp) {
                    send(client_fd, resp, resp_len, 0);
                    free(resp);
                }
            }
        }
        printf("上位机断开\n");
        close(client_fd);
        client_fd = -1;
    }
    return NULL;
}

static void* data_sender_func(void* arg) {
    int seq = 0;
    while (running) {
        if (measuring && client_fd >= 0) {
            uint8_t payload[2048];
            int payload_len = 0;
            if (encode_wind_field_data(payload, &payload_len) == 0) {
                int frame_len;
                uint8_t *frame = build_frame(0x0301, seq++, payload, payload_len, &frame_len);
                if (frame) {
                    send(client_fd, frame, frame_len, 0);
                    free(frame);
                }
            }
        }
        usleep(100000);
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
    pthread_t data_thread;
    pthread_create(&data_thread, NULL, data_sender_func, NULL);
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
