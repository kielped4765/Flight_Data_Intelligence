#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

// Windows networking headers (replaces sys/socket.h on Linux)
#include <winsock2.h>
#include <ws2tcpip.h>

#include "telemetry.h"
#include "parser.h"
#include "ring_buffer.h"

// Tell the linker to include the Winsock library
// This is Windows-only — on Linux you use -lsocket instead
#pragma comment(lib, "ws2_32.lib")

int main() {
    // ── 1. Initialize Winsock ────────────────────────────────
    // Windows requires WSAStartup before ANY socket call.
    // Linux has no equivalent — sockets work immediately.
    // MAKEWORD(2,2) requests Winsock version 2.2.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // ── 2. Create UDP socket ─────────────────────────────────
    // AF_INET     = IPv4
    // SOCK_DGRAM  = UDP (connectionless datagrams)
    // 0           = let the OS pick the protocol automatically
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // ── 3. Bind to port 5005 ─────────────────────────────────
    // sockaddr_in describes the address to listen on.
    // INADDR_ANY  = accept packets on any network interface.
    // htons()     = convert port number to network byte order.
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(5005);

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "bind() failed: " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // ── 4. Data structures ───────────────────────────────────
    RingBuffer<TelemetryFrame, 512> buffer;

    std::ofstream csv("data/telemetry.csv");
    if (!csv.is_open()) {
        std::cerr << "ERROR: Could not open data/telemetry.csv\n"
                  << "       Make sure the data/ directory exists.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    csv << "seq,timestamp_us,altitude_ft,airspeed_kts,heading_deg,"
           "engine_rpm,fuel_pct,latitude,longitude,fault_flags\n";

    // ── 5. Counters and timing ───────────────────────────────
    uint8_t  raw_buf[256];
    uint32_t good_count = 0;
    uint32_t bad_count  = 0;
    auto last_flush = std::chrono::steady_clock::now();

    std::cout << "AeroSentinel receiver listening on UDP :5005\n"
              << "TelemetryFrame size: " << sizeof(TelemetryFrame) << " bytes\n"
              << "Waiting for packets from simulator.py...\n";

    // ── 6. Main receive loop ─────────────────────────────────
    // recv() blocks until a UDP packet arrives.
    // On Windows the return type is int, not ssize_t like Linux.
    while (true) {
        int n = recv(sock, reinterpret_cast<char*>(raw_buf), sizeof(raw_buf), 0);
        if (n == SOCKET_ERROR) continue;

        TelemetryFrame frame;
        if (parse_bytes(raw_buf, static_cast<size_t>(n), frame)) {
            buffer.push(frame);
            ++good_count;
            std::cout << "[OK  #" << frame.sequence_number << "]"
                      << " alt="   << frame.altitude_ft
                      << " spd="   << frame.airspeed_kts
                      << " rpm="   << frame.engine_rpm
                      << " flags=" << static_cast<int>(frame.fault_flags)
                      << "\n";
        } else {
            ++bad_count;
            std::cerr << "[BAD] CRC mismatch — packet rejected"
                      << " (total bad: " << bad_count << ")\n";
        }

        // ── 7. Flush buffer to CSV every 5 seconds ───────────
        auto now     = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>
                       (now - last_flush).count();

        if (elapsed >= 5) {
            int flushed = 0;
            while (!buffer.empty()) {
                TelemetryFrame f = buffer.pop();
                csv << f.sequence_number  << ","
                    << f.timestamp_us     << ","
                    << f.altitude_ft      << ","
                    << f.airspeed_kts     << ","
                    << f.heading_deg      << ","
                    << f.engine_rpm       << ","
                    << f.fuel_pct         << ","
                    << f.latitude         << ","
                    << f.longitude        << ","
                    << static_cast<int>(f.fault_flags) << "\n";
                ++flushed;
            }
            csv.flush();
            last_flush = now;
            std::cout << "[CSV] Flushed " << flushed << " frames"
                      << " (total good: " << good_count << ")\n";
        }
    }

    // Cleanup (never reached in normal operation but good practice)
    closesocket(sock);
    WSACleanup();
    return 0;
}