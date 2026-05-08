import socket
import struct
import time
import math
import random
import argparse

# -- Packet format string ----------------------------
# Must match TelemetryFrame EXACTLY (same fields, same order, same types)

# '<' = little-endian (matches x86 C++ default)
# I = uint32_t (4 bytes) = sequence_number 
# Q = uint64_t (8 bytes) = timestamp_us
# 7f = 7 x float (4 bytes each) - alt, spd, hdg, rpm, fuel, lat, lon
# B = uint8_t (1 byte) - fault_flags
# H = uint16_t (2 bytes) - crc

# Total: 4+8+28+1+2 = 43 bytes (matches C++ static_assert)

PACKET_FMT = '<IQ7fBH'

def crc16(data: bytes) -> int:
    """CRC-16/CCITT-FALSE: same polynomial as C++ implementation."""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8) :
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
        crc &= 0xFFFF   # Keep only 16 bits
    return crc

def make_packet(seq: int, inject_fault: str | None) -> bytes:
    """
    Generate one telemetry packet with realistic oscillating values.
    inject_fault inserts one of four anomaly types for ML testing.
    """
    t = time.time()

    # Normal flight: gentle sinusoidal oscillation
    altitude    = 5000.0 + 200.0 * math.sin(t / 30.0)
    airspeed    = 250.0 + 10.0 * math.sin(t / 20.0)
    heading     = (t * 2.0) % 360.0
    engine_rpm  = 2400.0 + 50.0 * math.sin(t / 10.0)
    fuel_pct    = max(0.0, 80.0 - (t % 3600.0) / 3600.0 * 80.0)
    latitude    = 47.6062 + 0.001 * math.sin(t / 60.0)
    longitude   = -122.3321 + 0.001 * math.cos(t / 60.0)
    fault_flags = 0

    # ── Fault injection ──────────────────────────────────────
    # Each fault type represents a real scenario the ML must detect.
    # The fault_flags byte tells the pipeline what was injected
    # (used later to verify detection rate).
    if inject_fault == 'engine_spike':
        engine_rpm = 4800.0 + random.uniform(-50, 50)   # RPM >> normal
        fault_flags |= 0x01
    elif inject_fault == 'altitude_drop': 
        altitude -= 2000.0                              # Sudden 2000ft loss
    

        fault_flags |= 0x02
    elif inject_fault == 'gps_freeze':
        latitude = 47.6062                              # GPS locked (no change)
        longitude = -122.3321
        fault_flags |= 0x04
    elif inject_fault == 'sensor_dropout':
        altitude = airspeed = engine_rpm = 0.0          # All sensors zero
        fault_flags |= 0x08

    timestamp_us = int(t * 1_000_000)

    # -- Pack data without CRC first ---------------------------
    # struct.pack uses the same IEEE 754 representation as C++ float.
    body = struct.pack('<IQ7fB',
            seq, timestamp_us,
            altitude, airspeed, heading, engine_rpm, fuel_pct,
            latitude, longitude, fault_flags
    )
    
    # -- Compute and append CRC -------------------------------
    crc = crc16(body)
    return body + struct.pack('<H', crc)

def main():
    parser = argparse.ArgumentParser(description='AeroSentinel telemetry simulator')
    parser.add_argument(
        '--fault',
        choices=['engine_spike', 'altitude_drop', 'gps_freeze',
'sensor_dropout'],
        default=None,
        help='Continuously inject a specific fault type'

    )
    parser.add_argument(
        '--corrupt',
        action='store_true',
        help='Randomly corrupt 2%% of packets (tests CRC rejection)'
    )
    args = parser.parse_args()

    # UDP socket - no connection needed for datagrams
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    seq = 0

    print(f'Simulator running at 10Hz | fault={args.fault} |corrupt={args.corrupt}')
    print('Sending to 127.0.0.1:5005 ...')

    while True:
        packet = make_packet(seq, args.fault)

        # Optionally corrupt 2% of packets to test C++ CRC rejection
        if args.corrupt and random.random() < 0.02:
            packet = bytearray(packet)
            idx = random.randint(0, len(packet) - 3)    # Don't touch CRC itself
            packet[idx] ^= 0xFF                         # Flip all bits
            packet = bytes(packet)

        sock.sendto(packet, ('127.0.0.1', 5005))
        seq += 1
        time.sleep(0.1) # 10 Hz

if __name__ == '__main__':
    main()
