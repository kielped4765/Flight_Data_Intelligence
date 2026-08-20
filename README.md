# AeroSentinel 

[![CI](https://github.com/kielped4765/Flight_Data_Intelligence/actions/workflows/ci.yml/badge.svg)](https://github.com/kielped4765/Flight_Data_Intelligence/actions)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Python](https://img.shields.io/badge/Python-3.13-green)
![FastAPI](https://img.shields.io/badge/FastAPI-0.100+-009688)

> Real time avionics telemetry pipeline with ML-Powered anomaly detection
> A high-performance bridge between low-level C++ ingestion and high-level AI analysis.

---

## Setup to run the Program

This program was run on MSYM2 for the terminal windows

'''bash
## From the project root

Running & Testing AeroSentinel
Prerequisites

Make sure these are installed before starting:

MSYS2 MINGW64
Python 3.11+ with virtual environment
CMake 3.20+
Docker Desktop (for Grafana — optional)
Step 1 — Build the C++ receiver

## Step 1: Open MINGW64 Terminal 1 and run:

bash
cd /c/Users/peder/Flight_Data_intelligence/build
cmake .. -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles"
mingw32-make

Expected output: [100%] Built target Flight_Data_Intelligence

## Step 2 — Start the C++ receiver

In the same Terminal 1 run:

bash
./Flight_Data_Intelligence.exe

Expected output:

AeroSentinel receiver listening on UDP :5005
TelemetryFrame size: 43 bytes
Waiting for packets from simulator.py...

Leave this running.

## Step 3 — Start the Python simulator

Open MINGW64 Terminal 2 and run:

bash
cd /c/Users/peder/Flight_Data_intelligence
python scripts/simulator.py

Switch back to Terminal 1. You should immediately see:

[OK  #0] alt=4840.3 spd=248.1 rpm=2382.0 flags=0
[OK  #1] alt=4839.9 spd=248.2 rpm=2382.5 flags=0
[CSV] Flushed 50 frames (total good: 50)

Leave this running.

## Step 4 — Start the REST API

Open MINGW64 Terminal 3 and run:

bash
cd /c/Users/peder/Flight_Data_intelligence
python scripts/api.py

Expected output:

INFO:     Uvicorn running on http://0.0.0.0:8000

Leave this running.

## Step 5 — Test the API

Open MINGW64 Terminal 4 and run each command:

bash
# Public health check — no key needed
curl http://localhost:8000/status
# Expected: {"status":"online","service":"AeroSentinel","version":"1.0.0"}

# Live telemetry — requires API key
curl -H "X-API-Key: aerosentinel-dev-key" http://localhost:8000/telemetry/latest
# Expected: {"latest":{...current telemetry row...},"field_count":10}

# Anomaly log — requires API key
curl -H "X-API-Key: aerosentinel-dev-key" http://localhost:8000/anomalies
# Expected: {"count":0,"anomalies":[]}

# Verify wrong key is rejected
curl -H "X-API-Key: wrongkey" http://localhost:8000/telemetry/latest
# Expected: {"detail":"Invalid or missing API key. Include header: X-API-Key"}
Step 6 — Run unit tests

## In Terminal 4 run:

bash
cd /c/Users/peder/Flight_Data_intelligence/build
ctest --verbose

Expected output:

Test #1: ParserTest.ValidPacketParsesCorrectly    ... Passed
Test #2: ParserTest.CorruptedByteFailsCRC         ... Passed
Test #3: ParserTest.TooShortBufferReturnsFalse    ... Passed
Test #4: ParserTest.SequenceNumberPreserved       ... Passed
Test #5: RingBufferTest.PushPopSingleItem         ... Passed
Test #6: RingBufferTest.FIFOOrder                ... Passed
Test #7: RingBufferTest.OverflowOverwritesOldest  ... Passed
Test #8: RingBufferTest.PopOnEmptyThrows         ... Passed
Test #9: RingBufferTest.CapacityReported         ... Passed

100% tests passed, 0 tests failed
Step 7 — Test fault injection

<<<<<<< Updated upstream
=======
## In Terminal 2 stop the normal simulator with Ctrl+C then run:

bash
# Test engine spike detection
python scripts/simulator.py --fault engine_spike

# Test altitude drop detection
python scripts/simulator.py --fault altitude_drop

# Test GPS freeze detection
python scripts/simulator.py --fault gps_freeze

# Test sensor dropout detection
python scripts/simulator.py --fault sensor_dropout

# Test CRC rejection (corrupts 2% of packets)
python scripts/simulator.py --corrupt
## Security
See [SECURITY.md](SECURITY.md) for implementation notes and production
>>>>>>> Stashed changes
hardening checklist (JWT, mTLS, RBAC, FedRAMP considerations).

## Project Status

v1.0 — Complete. Core pipeline, binary-to-CSV ingestion, and ML-integrated API are fully functional and verified.
