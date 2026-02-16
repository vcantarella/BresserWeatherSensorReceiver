import os
import sys
import pytest
import json
import pandas as pd
import paho.mqtt.client as mqtt
from datetime import datetime, timedelta
import time
import duckdb
from unittest.mock import MagicMock, patch
import subprocess

server_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, server_dir)

from server_implementation import init_databases, process_data
from data_processor import get_raw_data, resample_data

@pytest.fixture(scope="session", autouse=True)
def mosquitto_service():
    # Start broker
    subprocess.run(["sudo", "systemctl", "start", "mosquitto", "-c",
                    os.path.join(server_dir, "mosquitto.conf")])
    time.sleep(2)  # Wait for broker to start
    
    yield
    
    # Cleanup
    subprocess.run(["sudo", "systemctl", "stop", "mosquitto"])
    time.sleep(2)  # Wait for broker to stop

@pytest.fixture
def mqtt_client(mosquitto_service):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    
    def on_connect(client, userdata, flags, rc, properties=None):
        print(f"Connected with result code {rc}")
    
    client.on_connect = on_connect
    client.connect("localhost", 1883, 60)
    client.loop_start()
    time.sleep(1)  # Wait for connection
    
    yield client
    
    client.loop_stop()
    client.disconnect()

@pytest.fixture
def test_data():
    return [
        {
            "sensor_id": "TEST001",
            "temperature": 20.0,
            "humidity": 65,
            "wind_speed": 2.5,
            "wind_direction": 180.0,
            "rain": 0.5,
            "rain_delta": 0.1,
            "light_lux": 1000.0,
            "delta_t": 300.0,
            "timestamp": datetime.now()
        },
        {
            "sensor_id": "TEST001",
            "temperature": 21.0,
            "humidity": 67,
            "wind_speed": 3.0,
            "wind_direction": 185.0,
            "rain": 1.0,
            "rain_delta": 0.5,
            "light_lux": 1100.0,
            "delta_t": 300.0,
            "timestamp": datetime.now() + timedelta(minutes=2)
        }
    ]

def test_full_pipeline(test_db_dir, test_data):
    raw_db = os.path.join(test_db_dir, "test_raw.duckdb")
    processed_db = os.path.join(test_db_dir, "test_processed.duckdb")
    
    # Initialize databases
    with patch('server_implementation.RAW_DB', raw_db), \
         patch('server_implementation.PROCESSED_DB', processed_db):
        init_databases()
        
        # Insert test data
        with duckdb.connect(raw_db) as conn:
            for data in test_data:
                conn.execute("""
                    INSERT INTO weather_data 
                    (id, sensor_id, temperature, humidity, wind_speed, 
                    wind_direction, rain, rain_delta, light_lux, delta_t, timestamp)
                    SELECT 
                        COALESCE((SELECT MAX(id) FROM weather_data), 0) + 1,
                        ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
                """, (
                    data['sensor_id'],
                    data['temperature'],
                    data['humidity'],
                    data['wind_speed'],
                    data['wind_direction'],
                    data['rain'],
                    data['rain_delta'],
                    data['light_lux'],
                    data['delta_t'],
                    data['timestamp']
                ))
        
        # Process data
        process_data()
        
        # Verify processed results
        with duckdb.connect(processed_db) as conn:
            df = conn.execute("""
                SELECT * FROM processed_intervals 
                ORDER BY timestamp
            """).fetchdf()
            
            # Verify resampled data
            assert len(df) > 0
            assert df['temperature'].mean() == pytest.approx(20.5, 0.1)
            assert df['rain_delta'].sum() == pytest.approx(0.6, 0.1)

def test_server_integration(mqtt_client, tmp_path):
    test_data = {
        "sensor_id": "TEST001",
        "temperature": 20.5,
        "humidity": 65,
        "wind_speed": 2.5,
        "wind_direction": 180.0,
        "rain": 0.5,
        "rain_delta": 0.1,
        "light_lux": 1000.0,
        "delta_t": 300.0
    }
    
    # Verify MQTT connection
    assert mqtt_client.is_connected()
    
    # Send test data
    result = mqtt_client.publish("weather/raw", json.dumps(test_data))
    result.wait_for_publish()
    
    time.sleep(2)  # Wait for processing
    
    # Check raw database
    with duckdb.connect("data/raw_weather.duckdb") as conn:
        raw_df = conn.execute("SELECT * FROM weather_data").fetchdf()
        assert len(raw_df) == 1
        
        # Verify data was stored correctly
        assert raw_df.iloc[0]['temperature'] == test_data['temperature']
        assert raw_df.iloc[0]['humidity'] == test_data['humidity']
    
    # Wait for processing cycle
    time.sleep(60)
    
    # Check processed database
    with duckdb.connect("data/processed_weather.duckdb") as conn:
        processed_df = conn.execute("""
            SELECT * FROM processed_intervals 
            ORDER BY timestamp DESC 
            LIMIT 1
        """).fetchdf()
        
        assert len(processed_df) > 0
        # Add specific assertions for processed data