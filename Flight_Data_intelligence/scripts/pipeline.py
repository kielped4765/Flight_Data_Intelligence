import time
import os
import joblib
import numpy as np
import pandas as pd

CSV_PATH      = 'data/telemetry.csv'
MODEL_PATH    = 'models/anomaly_model.joblib'
ANOMALY_LOG   = 'data/anomalies.log'
POLL_INTERVAL = 6

FEATURES = ['altitude_ft', 'airspeed_kts', 'engine_rpm', 
            'rate_of_climb', 'heading_change_rate']

def load_model():
    if os.path.exists(MODEL_PATH):
        model = joblib.load(MODEL_PATH)
        print(f'[SYSTEM] Anomaly model loaded from {MODEL_PATH}')
        return model
    print('[SYSTEM] No model found. Please run train_model.py first.')
    return None

def compute_derived_features(df: pd.DataFrame) -> pd.DataFrame:
    df = df.sort_values('timestamp_us').copy()
    df['dt'] = df['timestamp_us'].diff() / 1_000_000.0
    dt_safe = df['dt'].replace(0, np.nan)
    df['rate_of_climb'] = df['altitude_ft'].diff() / dt_safe
    df['heading_change_rate'] = df['heading_deg'].diff().abs() / dt_safe
    return df.fillna(0)

def score_batch(df: pd.DataFrame, model) -> tuple:
    X = df[FEATURES].values
    scores = model.decision_function(X)
    is_anomaly = (model.predict(X) == -1)
    return scores, is_anomaly

def log_anomalies(df: pd.DataFrame, scores, is_anomaly):
    anomaly_rows = df[is_anomaly]
    if len(anomaly_rows) == 0:
        return
        
    with open(ANOMALY_LOG, 'a') as f:
        for i, (_, row) in enumerate(anomaly_rows.iterrows()):
            idx = df.index.get_loc(row.name)
            f.write(f"{int(row['timestamp_us'])},{int(row['fault_flags'])},{scores[idx]:.4f}\n")
    
    print(f'[LOG] Wrote {len(anomaly_rows)} anomalies to {ANOMALY_LOG}')

def main():
    model = load_model()
    last_row = 0
    print(f'[START] Pipeline active. Watching {CSV_PATH}...')
    
    while True:
        try:
            df = pd.read_csv(CSV_PATH)
            new_rows = df.iloc[last_row:]
    
            if len(new_rows) > 0:
                batch = compute_derived_features(new_rows.copy())
                
                if model is not None:
                    scores, is_anomaly = score_batch(batch, model)
                else:
                    scores = np.zeros(len(batch))
                    is_anomaly = np.zeros(len(batch), dtype=bool)
                
                log_anomalies(batch, scores, is_anomaly)
                
                print(f'\n>>> [PIPELINE UPDATE]')
                print(f'    New Rows Processed: {len(new_rows)}')
                print(f'    Latest Altitude:    {batch["altitude_ft"].iloc[-1]:.1f} ft')
                print(f'    Anomalies Detected: {is_anomaly.sum()}')
                
                if is_anomaly.any():
                    print(f'    [!] WARNING: Unsafe flight parameters detected!')
                
                last_row = len(df)
                    
        except FileNotFoundError:
            pass
        except Exception as e:
            print(f'[ERROR] Pipeline encountered a problem: {e}')
                    
        time.sleep(POLL_INTERVAL)

if __name__ == '__main__':
    main()