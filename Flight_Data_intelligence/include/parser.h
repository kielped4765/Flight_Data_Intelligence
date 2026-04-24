#pragma once
#include "telemetry.h"
#include <cstdint> // size_t

// -- CRC-16 checksum ---------------------
// Computes a 16-bit cyclic redundancy check over 'length' bytes.
// Both the Python simulator and this C++ code use the same
// Polynomial (0x1021, CCITT-FALSE varient) so they agree.
uint16_t crc16(const uint8_t* data, size_t length);

// -- Packet validation -------------------
// Returns true if the frame;s embedded CRC matches a freshly
// computed CRC over all other fields.
bool validate_frame(const TelemetryFrame& frame);

// -- Raw bytes -> TelemetryFrame ---------
// Copies 'len' bytes from buf into out, then validates.
// Returns false if but is too small or CRC fails. 
bool parse_bytes(const uint8_t* buf, size_t len, TelemetryFrame& out);

// We seperate .h and .cpp because it is per translation unit
// each .cpp file is compiled independently. 