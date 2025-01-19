import duckdb
import os
from schema import WEATHER_SCHEMA, PROCESSED_SCHEMA

DB_DIR = 'data'
RAW_DB = os.path.join(DB_DIR, 'raw_weather.duckdb')
PROCESSED_DB = os.path.join(DB_DIR, 'processed_weather.duckdb')

def init_databases():
    if not os.path.exists(DB_DIR):
        os.makedirs(DB_DIR)
    
    # Initialize raw database
    with duckdb.connect(RAW_DB) as conn:
        conn.execute(WEATHER_SCHEMA)
    
    # Initialize processed database
    with duckdb.connect(PROCESSED_DB) as conn:
        conn.execute(PROCESSED_SCHEMA)

if __name__ == "__main__":
    init_databases()
    print(f"Databases initialized in {DB_DIR}")