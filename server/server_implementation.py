from fastapi import FastAPI
import duckdb
from paho.mqtt import client as mqtt_client
from datetime import datetime
import schedule
import time
import threading
import os
import logging
import uvicorn
import json
from data_processor import get_raw_data, resample_data, save_processed_data

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# MQTT Settings
MQTT_BROKER = 'localhost'  # Change from 0.0.0.0 to localhost
MQTT_PORT = 1883
MQTT_TOPIC = "weather/raw"
MQTT_RETRY_INTERVAL = 5  # seconds
MQTT_MAX_RETRIES = 5

app = FastAPI()
DB_DIR = 'data'
RAW_DB = os.path.join(DB_DIR, 'raw_weather.duckdb')
PROCESSED_DB = os.path.join(DB_DIR, 'processed_weather.duckdb')

def init_databases():
    if not os.path.exists(DB_DIR):
        os.makedirs(DB_DIR)
    
    with duckdb.connect(RAW_DB) as conn:
        conn.execute("""
            CREATE TABLE IF NOT EXISTS weather_data (
                id INTEGER PRIMARY KEY,
                sensor_id VARCHAR NOT NULL,
                temperature DOUBLE,
                humidity INTEGER,
                wind_speed DOUBLE,
                wind_direction DOUBLE,
                rain DOUBLE,
                rain_delta DOUBLE,
                light_lux DOUBLE,
                delta_t DOUBLE,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        """)
    
    with get_processed_db() as proc_conn:
        proc_conn.execute("""
            CREATE TABLE IF NOT EXISTS processed_weather (
                id INTEGER PRIMARY KEY,
                hour TIMESTAMP,
                avg_temperature DOUBLE,
                avg_humidity DOUBLE,
                max_wind_speed DOUBLE,
                total_rain DOUBLE,
                processed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        """)

def get_raw_db():
    return duckdb.connect(RAW_DB)

def get_processed_db():
    return duckdb.connect(PROCESSED_DB)


def clean_raw_database(raw_db):
    try:
        with duckdb.connect(raw_db) as conn:
            # Backup raw data first
            conn.execute("""
                CREATE TABLE IF NOT EXISTS weather_data_backup AS 
                SELECT * FROM weather_data
            """)
            # Clear raw data
            conn.execute("DELETE FROM weather_data")
            logger.info("Raw database cleaned after successful processing")
    except Exception as e:
        logger.error(f"Error during raw database cleanup: {e}")
        raise

def process_data():
    try:
        # Process data using imported functions
        raw_df = get_raw_data(RAW_DB)
        if raw_df.empty:
            logger.info("No new data to process")
            return

        processed_df = resample_data(raw_df)
        save_processed_data(processed_df, PROCESSED_DB)
        
        # Clean raw database after successful processing
        clean_raw_database(RAW_DB)
        logger.info("Data processing cycle completed")
        
    except Exception as e:
        logger.error(f"Failed to process data: {e}")

# Schedule processing every 30 minutes
schedule.every(30).minutes.do(process_data)

def run_scheduler():
    while True:
        schedule.run_pending()
        time.sleep(60)

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            logger.info("Connected to MQTT Broker!")
            client.subscribe(MQTT_TOPIC)
            logger.info(f"Subscribed to {MQTT_TOPIC}")
        else:
            logger.error(f"Failed to connect, return code {rc}\n")
            logger.error("0: Connection successful\n")
            logger.error("-1: Connection refused - incorrect protocol version\n")
            logger.error("-2: Connection refused - invalid client identifier\n")
            logger.error("-3: Connection refused - server unavailable\n")
            logger.error("-4: Connection refused - bad username or password\n")

    def on_message(client, userdata, msg):
        try:
            data = json.loads(msg.payload.decode())
            logger.info(f"Received weather data: {data}")
            with get_raw_db() as conn:
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
                    datetime.now()
                ))
                logger.info("Data inserted successfully")
        except KeyError as e:
            logger.error(f"Missing field in data: {e}")
        except Exception as e:
            logger.error(f"Error processing message: {e}")

    # Add client ID to avoid connection refused
    client = mqtt_client.Client(client_id="weather_server", protocol=mqtt_client.MQTTv311)
    client.on_connect = on_connect
    client.on_message = on_message

    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        return client
    except Exception as e:
        logger.error(f"MQTT Connection failed: {e}")
        return None

if __name__ == "__main__":
    init_databases()
    logger.info("Databases initialized")

    # Start MQTT client
    mqtt_client = connect_mqtt()
    if mqtt_client:
        mqtt_client.loop_start()
        logger.info("MQTT client started")

        # Start scheduler in background
        scheduler_thread = threading.Thread(target=run_scheduler, daemon=True)
        scheduler_thread.start()
        logger.info("Scheduler started")

        # Run FastAPI
        uvicorn.run(app, host="0.0.0.0", port=8000)