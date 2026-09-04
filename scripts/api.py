import os
import pandas as pd
from fastapi import FastAPI, HTTPException, Security, Depends
from fastapi.security import APIKeyHeader
from fastapi.middleware.cors import CORSMiddleware
from typing import Optional

# -- App setup ------------------------------
app = FastAPI(
    title='AeroSentinel API',
    description='Real-time avionics telemetry and anomaly detection API (Local Mode)',
    version='1.0.0'
)

# Allow dashboard or browser to call the API
app.add_middleware(
    CORSMiddleware,
    allow_origins=['*'],
    allow_methods=['GET', 'POST'],
    allow_headers=['*'],
)

# -- Configuration ----------------------------------
CSV_PATH    = 'data/telemetry.csv'
ANOMALY_LOG = 'data/anomalies.log'

# -- Authentication ----------------------------------
API_KEY     = os.environ.get('AERO_API_KEY', 'aerosentinel-dev-key')
api_key_hdr = APIKeyHeader(name='X-API-Key', auto_error=False)

def require_api_key(key: Optional[str] = Security(api_key_hdr)):
    """Validates the X-API-Key header."""
    if key != API_KEY:
        raise HTTPException(
            status_code=403,
            detail='Invalid or missing API key. Include header: X-API-Key'
        )
    return key

# -- Endpoints ------------------------------------------------

@app.get('/status', tags=['Health'])
def get_status():
    """Public health check — verify the API is alive."""
    return {'status': 'online', 'service': 'AeroSentinel', 'version': '1.0.0'}

@app.get('/telemetry/latest', tags=['Telemetry'])
def get_latest_telemetry(key: str = Depends(require_api_key)):
    """Reads the most recent row from the telemetry CSV."""
    try:
        if not os.path.exists(CSV_PATH):
            return {'latest': {}, 'message': 'Waiting for telemetry.csv...'}
        
        df = pd.read_csv(CSV_PATH)
        if df.empty:
            return {'latest': {}, 'message': 'Telemetry file is empty.'}
            
        # Get the very last row as a dictionary
        latest_data = df.iloc[-1].to_dict()
        return {'latest': latest_data, 'field_count': len(latest_data)}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Internal Server Error: {e}")

@app.get('/anomalies', tags=['Anomalies'])
def get_anomalies(limit: int = 50, key: str = Depends(require_api_key)):
    """Returns the last N detected anomalies from the local log."""
    try:
        anomalies = []
        if os.path.exists(ANOMALY_LOG):
            with open(ANOMALY_LOG, 'r') as f:
                lines = f.readlines()[-limit:]
                for line in lines:
                    parts = line.strip().split(',')
                    if len(parts) >= 3:
                        anomalies.append({
                            'timestamp_us': parts[0],
                            'fault_flags': parts[1],
                            'anomaly_score': parts[2]
                        })
        return {'count': len(anomalies), 'anomalies': anomalies}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Internal Server Error: {e}")

# -- Execution ------------------------------------------------
if __name__ == '__main__':
    import uvicorn
    # Start the server on port 8000
    uvicorn.run(app, host='0.0.0.0', port=8000)