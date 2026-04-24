#include "parser.h"
#include <cstring>  // for std::memcpy

// -- CRC-16/CCITT-FALSE ---------------------
//
// A CRC works by treating the input bytes as a very large number
// and computing its remainder when divided by a generator polynomial. 
// The polynomial 0x1021 is used by HDLC, X.25, SD cards, and many aerospace protocols.

// How to read this loop:
// For each byte, XOR it into the high byte of the accumulator. 
// Then shift through 8 bites if the high bit is set, the 'division'
// step XORs with the polynomial; otherwise just shift.

uint16_t crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;     // XOR into high byte.
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1; 
        }
    }
    return crc;
}

// -- Validate_frame ---------------------------
//
// The CRC in the struct covers ALL bytes EXCEPT the crc field itself. 
// We compute the CRC over (sizeof frame - 2) bytes, then compare
// to what the sender embedded. A mismatch means the packet was corrupted in transit.
// 
bool validate_frame(const TelemetryFrame& frame) {
    // Data length - everything before the crc field
    constexpr size_t data_len = sizeof(TelemetryFrame) - sizeof(uint16_t);
    uint16_t computed = crc16(
        reinterpret_cast<const uint8_t*>(&frame),
        data_len
    );
    return computed == frame.crc;
}

// -- parse_bytes -------------------------------
//
// Copies raw socket bytes into a TelemetryFrame, then validates.
// Returns false immediately if the buffer is too short.
//
bool parse_bytes(const uint8_t* buf, size_t len, TelemetryFrame& out) {
    if (len < sizeof(TelemetryFrame)) {
        return false;   // Truncated packet - reject
    }
    std::memcpy(&out, buf, sizeof(TelemetryFrame));
    return validate_frame(out);
}