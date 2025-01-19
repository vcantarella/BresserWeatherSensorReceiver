from datetime import datetime

WEATHER_SCHEMA = """
CREATE TABLE IF NOT EXISTS weather_data (
    id INTEGER PRIMARY KEY,
    sensor_id VARCHAR,
    temperature DOUBLE,
    humidity INTEGER,
    wind_speed DOUBLE,
    rain DOUBLE,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
)
"""

PROCESSED_SCHEMA = """
CREATE TABLE IF NOT EXISTS processed_weather (
    id INTEGER PRIMARY KEY,
    hour TIMESTAMP,
    avg_temperature DOUBLE,
    avg_humidity DOUBLE,
    max_wind_speed DOUBLE,
    total_rain DOUBLE,
    processed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
)
"""