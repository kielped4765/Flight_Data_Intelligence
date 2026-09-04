#include <gtest/gtest.h>
#include "parser.h"
#include "telemetry.h"
#include <cstring>
#include <vector>

// -- Helper: build a valid packet packet ---------------------------
// This creates a TelemetryFrame with known values and correct CRC,
// then incorporates it to a byte vector - exactly what recv() returns.
std::vector<uint8_t> make_valid_packet(uint32_t seq = 1) {
    TelemetryFrame f{};
    f.sequence_number = seq;
    f.timestamp_us = 1700000000000000ULL;
    f.altitude_ft = 5000.0f;
    f.airspeed_kts = 250.0f;
    f.heading_deg = 90.0f;
    f.engine_rpm = 2400.0f;
    f.fuel_pct = 75.0f;
    f.latitude = 47.6f;
    f.longitude = -122.3f;
    f.fault_flags = 0;
    // Compute and embed the correct CRC
    constexpr size_t data_len = sizeof(TelemetryFrame) - sizeof(uint16_t);
    f.crc = crc16(reinterpret_cast<const uint8_t*>(&f), data_len);
    // Copy struct bytes into a vector
    std::vector<uint8_t> buf(sizeof(TelemetryFrame));
    std::memcpy(buf.data(), &f, sizeof(TelemetryFrame));
    return buf;
}

// -- Test 1: A valid packet parses correctly ------------------
TEST(ParserTest, ValidPacketParsesCorrectly) {
    auto buf = make_valid_packet(42);
    TelemetryFrame out{};
    EXPECT_TRUE(parse_bytes(buf.data(), buf.size(), out));
    EXPECT_EQ(out.sequence_number, 42u);
    EXPECT_FLOAT_EQ(out.altitude_ft, 5000.0f);
    EXPECT_FLOAT_EQ(out.airspeed_kts, 250.0f);
}

// Test 2: A corrputed packet fails CRC -----------------------
TEST(ParserTest, CorruptedByteFailsCRC) {
    auto buf = make_valid_packet();
    buf[5] ^= 0xff;     // Flip all bits in byte 5
    TelemetryFrame out{};
    EXPECT_FALSE(parse_bytes(buf.data(), buf.size(), out));
}

// -- Test 3: A truncated buffer returns false ---------------
TEST(ParserTest, TooShortBufferReturnsFalse) {
    std::vector<uint8_t> buf(10, 0x00);     // Only 10 bytes (need 43)
    TelemetryFrame out{};
    EXPECT_FALSE(parse_bytes(buf.data(), buf.size(), out));
}

// -- Test 4: Sequence numbers round-trip correctly ----------
TEST(ParserTest, SequenceNumberPreserved) {
    for (uint32_t seq : {0u, 1u, 255u, 65535u, 4294967295u}) {
        auto buf = make_valid_packet(seq);
        TelemetryFrame out{};
        ASSERT_TRUE(parse_bytes(buf.data(), buf.size(), out));
        EXPECT_EQ(out.sequence_number, seq);
    }
}