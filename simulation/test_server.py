#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
雷达测试服务器
用于测试上位机所有功能

使用方法：
    python3 test_server.py

然后在上位机中连接 127.0.0.1:5000
"""

import socket
import struct
import threading
import time
import random

# 协议常量
FRAME_HEADER = 0xAA55
FRAME_TAIL = 0x55AA

# 命令码
CMD_HEARTBEAT = 0x0001
CMD_STATUS_QUERY = 0x0002
CMD_STATUS_REPORT = 0x0003
CMD_VERSION_QUERY = 0x0006
CMD_VERSION_REPORT = 0x0007
CMD_START_MEASURE = 0x0200
CMD_STOP_MEASURE = 0x0201
CMD_SWITCH_BEAM = 0x0202
CMD_WIND_DATA = 0x0301
CMD_ACK = 0x0080

# CRC16计算
def calc_crc16(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            crc = (crc << 1) ^ 0x1021 if crc & 0x8000 else crc << 1
            crc &= 0xFFFF
    return crc

# 构建帧
def build_frame(cmd, seq, payload=b''):
    frame = bytearray()
    frame.extend(struct.pack('>H', FRAME_HEADER))
    length = 2 + 4 + len(payload) + 2
    frame.extend(struct.pack('>H', length))
    frame.extend(struct.pack('>H', cmd))
    frame.extend(struct.pack('>I', seq))
    frame.extend(payload)
    crc = calc_crc16(bytes(frame[2:]))
    frame.extend(struct.pack('>H', crc))
    frame.extend(struct.pack('>H', FRAME_TAIL))
    return bytes(frame)

# 生成风场数据
def generate_wind_data():
    payload = bytearray()
    timestamp = int(time.time() * 1000000)
    payload.extend(struct.pack('>Q', timestamp))
    payload.append(0)  # 波束索引
    payload.extend(struct.pack('>H', 30))  # 距离门数
    payload.extend(struct.pack('>f', 10.0))  # 分辨率
    payload.extend(struct.pack('>f', 300.0))  # 最大距离
    payload.extend(b'\x00\x00\x00')  # 保留

    for i in range(30):
        speed = 5.0 + i * 0.3 + random.uniform(-0.5, 0.5)
        payload.extend(struct.pack('>f', speed))

    for i in range(30):
        direction = 180.0 + random.uniform(-10, 10)
        payload.extend(struct.pack('>f', direction))

    for i in range(30):
        vspeed = random.uniform(-0.5, 0.5)
        payload.extend(struct.pack('>f', vspeed))

    for i in range(30):
        conf = int(95 - i * 2 + random.uniform(-5, 5))
        payload.append(max(0, min(100, conf)))

    payload.extend(struct.pack('>f', 25.0))  # SNR
    payload.extend(struct.pack('>f', 0.1))   # 湍流

    return bytes(payload)

# 处理客户端
def handle_client(client_socket, addr):
    print(f"客户端连接: {addr}")
    seq = 0
    measuring = False

    try:
        while True:
            data = client_socket.recv(4096)
            if not data:
                break

            # 解析命令
            if len(data) >= 14:
                cmd = struct.unpack('>H', data[4:6])[0]
                seq_num = struct.unpack('>I', data[6:10])[0]
                print(f"收到命令: 0x{cmd:04X}")

                # 响应
                if cmd == CMD_HEARTBEAT:
                    resp = build_frame(CMD_ACK, seq_num)
                    client_socket.sendall(resp)
                    print("发送心跳响应")

                elif cmd == CMD_STATUS_QUERY:
                    status = bytearray()
                    status.append(2)  # state
                    status.extend(struct.pack('>f', 35.0))  # temperature
                    status.extend(struct.pack('>f', 12.0))  # voltage
                    resp = build_frame(CMD_STATUS_REPORT, seq_num, bytes(status))
                    client_socket.sendall(resp)
                    print("发送状态响应")

                elif cmd == CMD_VERSION_QUERY:
                    version = b'1.0.0 Test Server'
                    resp = build_frame(CMD_VERSION_REPORT, seq_num, version)
                    client_socket.sendall(resp)
                    print("发送版本响应")

                elif cmd == CMD_START_MEASURE:
                    measuring = True
                    resp = build_frame(CMD_ACK, seq_num)
                    client_socket.sendall(resp)
                    print("开始测量")

                elif cmd == CMD_STOP_MEASURE:
                    measuring = False
                    resp = build_frame(CMD_ACK, seq_num)
                    client_socket.sendall(resp)
                    print("停止测量")

                elif cmd == CMD_SWITCH_BEAM:
                    beam = data[10] if len(data) > 10 else 0
                    resp = build_frame(CMD_ACK, seq_num)
                    client_socket.sendall(resp)
                    print(f"切换波束: {beam}")

            # 发送风场数据
            if measuring:
                wind_data = generate_wind_data()
                resp = build_frame(CMD_WIND_DATA, seq, wind_data)
                client_socket.sendall(resp)
                seq += 1

            time.sleep(0.1)  # 10Hz

    except Exception as e:
        print(f"错误: {e}")
    finally:
        print(f"客户端断开: {addr}")
        client_socket.close()

# 主函数
def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(('0.0.0.0', 5000))
    server.listen(5)

    print("=" * 50)
    print("雷达测试服务器")
    print("=" * 50)
    print("监听端口: 5000")
    print("在上位机中连接 127.0.0.1:5000")
    print("按 Ctrl+C 停止")
    print("=" * 50)

    try:
        while True:
            client, addr = server.accept()
            thread = threading.Thread(target=handle_client, args=(client, addr))
            thread.daemon = True
            thread.start()
    except KeyboardInterrupt:
        print("\n服务器已停止")

if __name__ == '__main__':
    main()
