#!/usr/bin/env python3
"""Five-beam wind-profiler simulator for the desktop upper computer."""

from __future__ import annotations

import argparse
import math
import random
import socket
import struct
import time
from dataclasses import dataclass


FRAME_HEADER = 0xAA55
FRAME_TAIL = 0x55AA

RESPONSE_SUCCESS = 0x0000
QUERY_DEVICE_INFO = 0x0100
QUERY_WIND_PROFILE = 0x0101
QUERY_RADIAL_SCAN = 0x0106
START_MEASURE = 0x0200
STOP_MEASURE = 0x0201
PUSH_WIND_PROFILE = 0x8100
PUSH_RADIAL_RAY = 0x8105

GATE_COUNT = 30
HEIGHT_SPACING_M = 10.0
FIRST_HEIGHT_M = 10.0
FIELD_MASK = 0x37  # radial velocity, CNR, spectrum width, confidence, quality flags


@dataclass(frozen=True)
class BeamGeometry:
    beam_id: int
    name: str
    azimuth_deg: float
    elevation_deg: float
    carrier_hz: float


# Elevation is measured up from the horizontal plane. The inclined beams are
# 15 degrees away from vertical, hence elevation=75 degrees.
BEAMS = (
    BeamGeometry(0, "VERT", 0.0, 90.0, 23.8e9),
    BeamGeometry(1, "NE", 45.0, 75.0, 23.9e9),
    BeamGeometry(2, "SE", 135.0, 75.0, 24.0e9),
    BeamGeometry(3, "SW", 225.0, 75.0, 24.1e9),
    BeamGeometry(4, "NW", 315.0, 75.0, 24.2e9),
)


def crc16_modbus(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc & 0xFFFF


def build_frame(command: int, sequence: int, payload: bytes = b"") -> bytes:
    length = 2 + 4 + len(payload) + 2
    prefix = struct.pack(">HHHI", FRAME_HEADER, length, command, sequence) + payload
    return prefix + struct.pack(">HH", crc16_modbus(prefix), FRAME_TAIL)


def extract_frames(buffer: bytearray) -> list[tuple[int, int, bytes]]:
    frames: list[tuple[int, int, bytes]] = []
    marker = struct.pack(">H", FRAME_HEADER)
    while True:
        position = buffer.find(marker)
        if position < 0:
            if len(buffer) > 1:
                del buffer[:-1]
            return frames
        if position:
            del buffer[:position]
        if len(buffer) < 14:
            return frames
        length = struct.unpack_from(">H", buffer, 2)[0]
        frame_size = 4 + length + 2
        if frame_size < 14 or frame_size > 4096:
            del buffer[0]
            continue
        if len(buffer) < frame_size:
            return frames
        frame = bytes(buffer[:frame_size])
        del buffer[:frame_size]
        stored_crc, tail = struct.unpack_from(">HH", frame, frame_size - 4)
        if tail != FRAME_TAIL or stored_crc != crc16_modbus(frame[:-4]):
            print("丢弃CRC或帧尾错误的客户端帧")
            continue
        command, sequence = struct.unpack_from(">HI", frame, 4)
        frames.append((command, sequence, frame[10:-4]))


def wind_truth(height_m: float, elapsed_s: float) -> tuple[float, float, float]:
    """Return an ENU wind vector with realistic shear, veer and vertical motion."""
    speed = 5.8 + 0.010 * height_m + 0.65 * math.sin(elapsed_s / 24.0 + height_m / 130.0)
    direction_from_deg = 218.0 + 0.055 * height_m + 4.0 * math.sin(elapsed_s / 50.0)
    direction_rad = math.radians(direction_from_deg)
    eastward = -speed * math.sin(direction_rad)
    northward = -speed * math.cos(direction_rad)
    upward = 0.28 * math.sin(elapsed_s / 18.0 + height_m / 85.0)
    return eastward, northward, upward


def radial_velocity(beam: BeamGeometry, vector: tuple[float, float, float]) -> float:
    azimuth = math.radians(beam.azimuth_deg)
    elevation = math.radians(beam.elevation_deg)
    eastward, northward, upward = vector
    return (
        eastward * math.cos(elevation) * math.sin(azimuth)
        + northward * math.cos(elevation) * math.cos(azimuth)
        + upward * math.sin(elevation)
    )


def generate_scan(scan_id: int, started_monotonic: float) -> tuple[list[bytes], bytes, tuple[float, float, float]]:
    elapsed = time.monotonic() - started_monotonic
    heights = [FIRST_HEIGHT_M + index * HEIGHT_SPACING_M for index in range(GATE_COUNT)]
    truth = [wind_truth(height, elapsed) for height in heights]
    rng = random.Random(scan_id)
    ray_payloads: list[bytes] = []
    all_cnr: list[float] = []

    for ray_index, beam in enumerate(BEAMS):
        elevation_rad = math.radians(beam.elevation_deg)
        start_range = FIRST_HEIGHT_M / math.sin(elevation_rad)
        gate_spacing = HEIGHT_SPACING_M / math.sin(elevation_rad)
        radial_values: list[float] = []
        cnr_values: list[float] = []
        widths: list[float] = []
        confidences: list[int] = []
        quality_flags: list[int] = []

        for gate, (height, vector) in enumerate(zip(heights, truth)):
            noise_sigma = 0.025 + height / 18000.0
            radial_values.append(radial_velocity(beam, vector) + rng.gauss(0.0, noise_sigma))
            cnr = 19.0 - 0.035 * height + 1.2 * math.sin(elapsed / 14.0 + gate / 5.0 + ray_index)
            cnr_values.append(cnr)
            all_cnr.append(cnr)
            widths.append(0.16 + 0.05 * abs(math.sin(height / 90.0 + elapsed / 30.0)))
            confidence = max(0, min(100, round(96.0 - max(0.0, 8.0 - cnr) * 2.5)))
            confidences.append(confidence)
            quality_flags.append(0 if confidence >= 50 else 1)

        header = struct.pack(
            "<HHHBBffffffdIIII",
            ray_index,
            len(BEAMS),
            GATE_COUNT,
            beam.beam_id,
            0,
            beam.azimuth_deg,
            beam.elevation_deg,
            start_range,
            gate_spacing,
            20000.0,
            15.0,
            beam.carrier_hz,
            1,
            FIELD_MASK,
            0,
            0,
        )
        arrays = bytearray()
        arrays.extend(struct.pack(f"<{GATE_COUNT}h", *(round(value * 100.0) for value in radial_values)))
        arrays.extend(struct.pack(f"<{GATE_COUNT}h", *(round(value * 100.0) for value in cnr_values)))
        arrays.extend(struct.pack(f"<{GATE_COUNT}H", *(round(value * 100.0) for value in widths)))
        arrays.extend(bytes(confidences))
        arrays.extend(struct.pack(f"<{GATE_COUNT}H", *quality_flags))
        ray_payloads.append(header + arrays)

    speeds: list[float] = []
    directions: list[float] = []
    upward_values: list[float] = []
    confidences: list[int] = []
    for height, (eastward, northward, upward) in zip(heights, truth):
        speeds.append(math.hypot(eastward, northward))
        directions.append(math.degrees(math.atan2(-eastward, -northward)) % 360.0)
        upward_values.append(upward)
        confidences.append(max(50, min(100, round(96.0 - height / 30.0))))

    legacy = bytearray()
    legacy.extend(struct.pack("<Q", int(time.time() * 1000)))
    legacy.append(0)
    legacy.extend(struct.pack(">H", GATE_COUNT))
    legacy.extend(struct.pack("<ff", HEIGHT_SPACING_M, heights[-1]))
    legacy.extend(b"\x00\x00\x00")
    legacy.extend(struct.pack(f"<{GATE_COUNT}f", *speeds))
    legacy.extend(struct.pack(f"<{GATE_COUNT}f", *directions))
    legacy.extend(struct.pack(f"<{GATE_COUNT}f", *upward_values))
    legacy.extend(bytes(confidences))
    legacy.extend(struct.pack("<ff", sum(all_cnr) / len(all_cnr), 0.08))
    return ray_payloads, bytes(legacy), truth[0]


def send_products(client: socket.socket, scan_id: int, started_monotonic: float, include_legacy: bool = True) -> None:
    rays, legacy, first_truth = generate_scan(scan_id, started_monotonic)
    if include_legacy:
        client.sendall(build_frame(PUSH_WIND_PROFILE, scan_id, legacy))
    for payload in rays:
        client.sendall(build_frame(PUSH_RADIAL_RAY, scan_id, payload))
    eastward, northward, upward = first_truth
    print(
        f"扫描 {scan_id}: 五束×{GATE_COUNT}门, "
        f"首层真值 u={eastward:.2f}, v={northward:.2f}, w={upward:.2f} m/s"
    )


def serve_client(client: socket.socket, address: tuple[str, int], started_monotonic: float) -> None:
    print(f"客户端连接: {address[0]}:{address[1]}")
    client.settimeout(0.2)
    receive_buffer = bytearray()
    measuring = False
    radial_mode = False
    scan_id = 1
    next_push = time.monotonic()
    try:
        while True:
            try:
                chunk = client.recv(4096)
                if not chunk:
                    break
                receive_buffer.extend(chunk)
            except socket.timeout:
                pass

            for command, sequence, _ in extract_frames(receive_buffer):
                print(f"收到命令: 0x{command:04X}, sequence={sequence}")
                if command == START_MEASURE:
                    measuring = True
                    next_push = time.monotonic()
                    client.sendall(build_frame(RESPONSE_SUCCESS, sequence))
                elif command == STOP_MEASURE:
                    measuring = False
                    client.sendall(build_frame(RESPONSE_SUCCESS, sequence))
                elif command == QUERY_WIND_PROFILE:
                    _, legacy, _ = generate_scan(scan_id, started_monotonic)
                    client.sendall(build_frame(PUSH_WIND_PROFILE, scan_id, legacy))
                elif command == QUERY_RADIAL_SCAN:
                    radial_mode = True
                    send_products(client, scan_id, started_monotonic, include_legacy=False)
                    scan_id += 1
                elif command == QUERY_DEVICE_INFO:
                    client.sendall(build_frame(RESPONSE_SUCCESS, sequence))
                else:
                    client.sendall(build_frame(RESPONSE_SUCCESS, sequence))

            now = time.monotonic()
            if measuring and now >= next_push:
                send_products(client, scan_id, started_monotonic, include_legacy=not radial_mode)
                scan_id += 1
                next_push = now + 1.0
    except (ConnectionError, OSError) as exc:
        print(f"连接结束: {exc}")
    finally:
        client.close()
        print(f"客户端断开: {address[0]}:{address[1]}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Five-beam wind-profiler simulator")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", default=5000, type=int)
    args = parser.parse_args()
    started_monotonic = time.monotonic()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((args.host, args.port))
        server.listen(5)
        print("测风雷达五波束仿真器")
        print(f"监听 {args.host}:{args.port}")
        print("波束: 法向90° + 45/135/225/315°方位、75°仰角")
        while True:
            try:
                client, address = server.accept()
                serve_client(client, address, started_monotonic)
            except KeyboardInterrupt:
                print("\n仿真器已停止")
                break


if __name__ == "__main__":
    main()
