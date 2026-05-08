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

## 1. Initialize Python Environment
python -m venv .venv
source .venv/Scripts/activate
./.venv/Scripts/python.exe -m pip install pandas fastapi uvicorn joblib scikit-learn numpy

## 2. Build the C++ Receiver
'''bash
mkdir -p data build
cd build
cmake .. -DCMAKE_BUILD_TYPE=DEBUG -G "MinGW Makefiles"
mingw32-make 

## Running the Pipeline (4 Terminal Sequence)
1. C++ Receiver           ./build/Flight_Data_Intelligence.exe
2. Simulator              ./.venv/Scripts/python.exe 
                            scripts/simulator.py
3. FastAPI                ./.venv/Scripts/python.exe
                            scripts/api.py
4. ML Pipeline            ./.venv/Scripts/python.exe
                            scripts/pipeline.py

## Architecture & Data Flow
1. Ingestion: Flight_Data_Intelligence.exe (C++) listens on UDP :5005. It validates binary packets using CRC-16/CCITT.

2. Storage: Validated frames are stored in a thread-safe Ring Buffer and flushed to data/telemetry.csv.

3. Analysis: pipeline.py monitors the CSV, computes derived features, and runs an Isolation Forest model to detect anomalies.

4. Access: api.py serves the latest telemetry and detected anomalies via a REST API with Swagger documentation.

## How this looks with the files:

simulator.py (Python)
|
| UDP binary packets (10 Hz, CRC-16 validated)
v
aerosentinel (C++17)
| -- UDP receiver, ring buffer, CSV writer
v
data/telemetry.csv
|
v
pipeline.py (Python)
| -- pandas, Isolation Forest, derived features
v
InfluxDB <──── Grafana (live dashboard)
|
v
api.py / FastAPI ──── /docs (Swagger UI)

## Tech stack
| Layer | Technology | Purpose |
|-------|------------|---------|
| Telemetry engine | C++17, CMake, POSIX sockets | High-performance binary
protocol handling |
| Data validation | CRC-16/CCITT, Google Test, ASan | Packet integrity, unit
tests, memory safety |
| Analytics | Python, pandas, numpy | Feature engineering, data
transformation |
| ML | scikit-learn Isolation Forest | Unsupervised anomaly detection |
| Storage | InfluxDB 2.x (Flux) | Time-series sensor data |
| API | FastAPI, Pydantic, Uvicorn | REST endpoints, OpenAPI docs |
| Visualization | Grafana | Live telemetry dashboard |
| DevOps | Docker Compose, GitHub Actions | Reproducible deployment, CI |


## Fault injection & anomaly detection
The simulator supports four injected fault types for testing:
```bash
python3 scripts/simulator.py --fault engine_spike # RPM >> normal
python3 scripts/simulator.py --fault altitude_drop # Sudden 2000ft loss
python3 scripts/simulator.py --fault gps_freeze # GPS position locked
python3 scripts/simulator.py --fault sensor_dropout # All sensors zero
```
The Isolation Forest model (trained on clean data) detects all four.
Anomalies appear as red markers on the Grafana dashboard in real time.

## Testing and Anomaly Detection

1. Unit Tests (C++)

'''bash
cd build
ctest --verbose

2. Anomaly Detection Test

'''bash
In terminal 2...
./.venv/Scripts/python.exe scripts/simulator.py --fault engine_spike

Then check http://localhost:8000/docs to verify

## Security
See [SECURITY.md](SECURITY.md) for implementation notes and production
hardening checklist (JWT, mTLS, RBAC, FedRAMP considerations).

## Project Status

v1.0 — Complete. Core pipeline, binary-to-CSV ingestion, and ML-integrated API are fully functional and verified.