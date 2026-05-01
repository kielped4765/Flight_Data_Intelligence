import os
import pandas as pd
import numpy as np
import joblib
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import Pipeline

# -- Configuration -----------------
CSV_PATH    = 'data/telemetry.csv'
MODEL_PATH  = 'models/anomaly_model.joblib'
FEATURES    = ['altitude_ft', 'airspeed_kts', 'enginge_rpm',
               'rate_of_climb', 'heading_change_rate']

def compute_derived(df: pd.DataFrame) -> pd.DataFrame:
    """Same feature engineering as pipeline.py - must be identical."""
    df = df.sort_values('timestamp_us').copy()
    df['dt'] = df['timestamp_us'].diff() / 1_000_000.0
    dt_safe = df['dt'].replace(0, np.nan)
    df['rate_of_climb']         = df['altitude_ft'].diff() / dt_safe
    df['heading_change_rate']   = df['heading_deg'].diff().abs() / dt_safe
    return df.fillna(0)

def main():
    # -- 1. Load training data -------------------------------
    print(f'Loading training data from {CSV_Path} ...')
    df = pd.read_csv(CSV_PATH)
    print(f'Loaded {len(df):,} rows')

    if len(df) < 1000:
        print('WARNING: Less than 1000 rows. Run the simulator for at least')
        print('5 minutes before training for reliable results.')

    # -- 2. Feature engineering ------------------------------
    df  = compute_derived(df)
    X   = df[FEATURES].values
    print(f'Feature matrix shape: {X.shape}')

    # -- 3. Build and train the model pipeline ----------------

    # StandardScaler: transforms each feature to mean=0, std=1.
    # This is critical for Isolation Forest - unscaled features
    # with different magnitudes (e.g. altitude in thousands vs 
    # fuel_pot in 0-100) would bias the random splits.

    # IsolationForest:
    #   n_estimators=200:   number of isolation trees (more = stabler)
    #   contamination=0.02: expected fraction of anomalies in data (2%)
    #   random_state=42:    seed for reproducibility 

    model = Pipeline([
        ('scaler', StandardScaler()),
        ('clf',    IsolationForest(
            n_estimators=200,
            contamination=0.02,
            random_state=42
        ))
    ])

    print('Training Isolation Forest ...')
    model.fit(X)
    print('Training complete.')

    # -- 4. Sanity check on training data ----------------------
    preds = model.predict(X)
    anomaly_rate = (preds == -1).mean()
    print(f'Anomaly rate on training data: {anomaly_rate:.1%}')
    print(f'Expected: ~2.0% (matches contamination parameter)')

    if anomaly_rate > 0.10:
        print('WARNING: >10% anomaly rate. Your training data may contain')
        print('faults. Re-generate with simulator.py (no --fault flag).')

    # -- 5. Save model ------------------------------------------
    os.makedirs('models', exist_ok=True)
    joblib.dump(model, MODEL_PATH)
    print(f'Model saved to {MODEL_PATH}')

    # -- 6. Feature importance (IsoloationForest doesn't have
    #       featuure_importances_ directly, but we can check mean
    #       depth as a proxy) -----------------------------------
    print('\nFeature summary (training set statistics):')
    for feat in FEATURES:
        col = df[feat]
        print(f'    {feat:25s}  mean={col.mean():8.2f}  std={col.std():8.2f}')

if __name__ == '__main__':
    main()