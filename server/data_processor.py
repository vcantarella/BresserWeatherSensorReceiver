import pandas as pd
import duckdb
from datetime import datetime, timedelta

def get_raw_data(raw_db):
    with duckdb.connect(raw_db) as conn:
        df = conn.execute("SELECT * FROM weather_data").fetchdf()
    df['timestamp'] = pd.to_datetime(df['timestamp'])
    return df.set_index('timestamp')

def resample_data(df, interval='5min'):
    df.loc[df.rain_delta < 0, "rain_delta"] = 0.0
    resampled = df.resample(interval).agg({
        'temperature': 'mean',
        'humidity': 'mean',
        'wind_speed': 'mean',
        'wind_direction': 'mean',
        'rain': 'sum',
        'rain_delta': 'sum',
        'light_lux': 'mean',
        'delta_t': 'mean'
    })
    return resampled.ffill()

def save_processed_data(df, processed_db):
    with duckdb.connect(processed_db) as conn:
        # Create table if not exists
        conn.execute("""
            CREATE TABLE IF NOT EXISTS processed_intervals (
                timestamp TIMESTAMP,
                temperature DOUBLE,
                humidity DOUBLE,
                wind_speed DOUBLE,
                wind_direction DOUBLE,
                rain DOUBLE,
                rain_delta DOUBLE,
                light_lux DOUBLE,
                delta_t DOUBLE
            )
        """)
        # Insert processed data
        records = df.reset_index().to_records(index=False)
        conn.executemany(
            "INSERT INTO processed_intervals VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
            records.tolist()
        )


# def process_weather_data(raw_db, processed_db):
#     try:
#         # Get and process data
#         raw_data = get_raw_data(raw_db)
#         if raw_data.empty:
#             logger.info("No new data to process")
#             return
            
#         processed_data = resample_data(raw_data)
        
#         # Save processed data
#         save_processed_data(processed_data, processed_db)
        
#         # Clean raw database after successful processing
#         clean_raw_database(raw_db)
        
#         logger.info("Data processing and cleanup completed successfully")
        
#     except Exception as e:
#         logger.error(f"Error during data processing: {e}")
#         raise