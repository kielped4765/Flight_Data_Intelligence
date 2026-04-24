#pragma once
#include <cstdint>

// ── Telemetry packet structure ───────────────────────────────
//
// #pragma pack(push, 1) forces the compiler to place struct fields
// with NO padding bytes between them. Without this, a compiler
// might insert 3 bytes after a uint8_t to align the next float
// to a 4-byte boundary. In binary protocols both sides MUST
// agree on the exact byte layout.
//
#pragma pack(push, 1)
struct TelemetryFrame { 
    uint32_t sequence_number;   // 4 bytes: packet counter
    uint64_t timestamp_us;      // 8 bytes: microseconds since epoch
    float    altitude_ft;       // 4 bytes: altitude in feet
    float    airspeed_kts;      // 4 bytes: indicated airspeed in knots
    float    heading_deg;       // 4 bytes: magnetic heading 0-360
    float    engine_rpm;        // 4 bytes: engine RPM
    float    fuel_pct;          // 4 bytes: fuel level 0-100
    float    latitude;          // 4 bytes: GPS latitude
    float    longitude;         // 4 bytes: GPS longitude
    uint8_t  fault_flags;       // 1 byte: bit flags for injected faults
    uint16_t crc;               // 2 bytes: CRC-16 checksum
                                // TOTAL: 43 bytes
};
#pragma pack(pop)

//Compile-time size check — build fails if struct layout changes
static_assert(sizeof(TelemetryFrame) == 43,
"TelemetryFrame size mismatch: check packing");