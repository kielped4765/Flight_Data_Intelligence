import time
import os
import joblib
import numpy as np
import pandas as pd

# -- Configuration ------------------------------------
# In production, use environment variables or a secrets manager.
# Never hardcode tokens in source code that get committed.
CSV_PATH        = 'data/telemetry.csv'
MODEL_PATH      = 'models/anomaly_model.joblib'
ANOMALY_LOG     = 'data/anomalies.log'
POLL_INTERVAL   = 6     # seconds between CSV checks

# -- Feature list used by the ML model ----------------------------
# These must match exactly what train_model.py uses.
# Any change here breaks the model's expectations.
FEATURES = ['altitude_ft', 'airspeed_kts', 'engine_rpm',
            'rate_of_climb', 'heading_change_rate']