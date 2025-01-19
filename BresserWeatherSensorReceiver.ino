///////////////////////////////////////////////////////////////////////////////////////////////////
// BresserWeatherSensorBasic.ino
// My version of the Bresser Weather Sensor Receiver
// Edited from Matthias Prinke's BresserWeatherSensorBasic.ino
// Example for BresserWeatherSensorReceiver - 
// Using getMessage() for non-blocking reception of a single data message.
//
// The data may be incomplete, because certain sensors need two messages to
// transmit a complete data set.
// Which sensor data is received in case of multiple sensors are in range
// depends on the timing of transmitter and receiver.  
//
// https://github.com/matthias-bs/BresserWeatherSensorReceiver
//
//
// created: 05/2022
//
//
// MIT License
//
// Copyright (c) 2022 Matthias Prinke
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// History:
//
// 20220523 Created from https://github.com/matthias-bs/Bresser5in1-CC1101
// 20220524 Moved code to class WeatherSensor
// 20220810 Changed to modified WeatherSensor class; fixed Soil Moisture Sensor Handling
// 20220815 Changed to modified WeatherSensor class; added support of multiple sensors
// 20221227 Replaced DEBUG_PRINT/DEBUG_PRINTLN by Arduino logging functions
// 20230624 Added Bresser Lightning Sensor decoder
// 20230804 Added Bresser Water Leakage Sensor decoder
// 20231023 Modified detection of Lightning Sensor
// 20231025 Added Bresser Air Quality (Particulate Matter) Sensor decoder
// 20240209 Added Leakage, Air Quality (HCHO/VOC) and CO2 Sensors
// 20240213 Added PM1.0 to Air Quality (Particulate Matter) Sensor decoder
// 20240716 Fixed output of invalid battery state with 6-in-1 decoder
// 20241228 Simplified for the sensor in Kassel
// ToDo: 
// - 
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "src/WeatherSensorCfg.h"
#include "src/WeatherSensor.h"
#include "src/Decoder.h"
#include <WiFi.h>
#include <time.h>
#include <RadioLib.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "src/Secrets.h"

#define JSON_BUFFER_SIZE 300

bool publishWeatherData(weather_data_t *ws, PubSubClient& mqtt_client, const char* mqtt_topic) {
    if (!mqtt_client.connected()) {
        Serial.println("MQTT client not connected");
        return false;
    }

    StaticJsonDocument<300> doc;
    char time_str[26];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &ws->timestamp);
    
    doc["sensor_id"] = ws->sensor_id;
    doc["temperature"] = ws->temp_c;
    doc["humidity"] = ws->humidity;
    doc["wind_speed"] = ws->wind_avg_meter_sec;
    doc["wind_direction"] = ws->wind_direction_deg;
    doc["rain"] = ws->rain_mm;
    doc["rain_delta"] = ws->delta_rain;
    doc["light_lux"] = ws->light_lux;
    doc["timestamp"] = time_str;
    doc["delta_t"] = ws->delta_t;
    
    char buffer[JSON_BUFFER_SIZE];
    serializeJson(doc, buffer);
    
    bool published = mqtt_client.publish(mqtt_topic, buffer);
    
    if (!published) {
        Serial.println("Failed to publish MQTT message");
    }
    
    return published;
}

// Add MQTT Configuration
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER; // Computer's IP
const int mqtt_port = MQTT_PORT;
const char* mqtt_topic = "weather/raw";

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void setup_wifi() {
    delay(10);
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
}

void setup_mqtt() {
    mqtt_client.setServer(mqtt_server, mqtt_port);
    while (!mqtt_client.connected()) {
        Serial.println("Connecting to MQTT...");
        if (mqtt_client.connect("MDF", MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("MQTT connected");
        } else {
            Serial.println("failed, rc=");
            Serial.print(mqtt_client.state());
            delay(5000);
        }
    }
}

// NTP Server configuration for Germany
const char* ntpServer = "de.pool.ntp.org";  // German NTP server pool
const long  gmtOffset_sec = 3600;           // UTC+1 (German standard time)
const int   daylightOffset_sec = 3600;      // +1 hour for summer time

// Flag to indicate that a packet was received
volatile bool receivedFlag = false;
SX1276 radio = new Module(PIN_RECEIVER_CS, PIN_RECEIVER_IRQ, PIN_RECEIVER_RST, PIN_RECEIVER_GPIO);
weather_data_t ws;
sensor_t sensor;
// timeout time
const uint32_t timeout = 10000;

// C-style interrupt handler
void setReceivedFlag(void) {
    receivedFlag = true;
}
void setFlag(void)
{
    // We got a packet, set the flag
    receivedFlag = true;
}

float previous_rain = 0.0; // we initialize a rain variable that is updated as new data arrives.
// The data arrives as the accumulated rain and we want the rain difference.
struct tm previous_time; // we compare the delta t for the rain intensity.

void setup() 
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.printf("Starting execution...\n");
    // Connect to WiFi
    setup_wifi();
    setup_mqtt();
    
    //initialize wifi access
    double frequency_offset = 0.0;
    double frequency = 868.3 + frequency_offset;
    log_d("Setting frequency to %f MHz", 868.3 + frequency_offset);
    int state = radio.beginFSK(frequency, 8.21, 57.136417, 250, 10, 32);
    log_d("%s Initializing ... ", RECEIVER_CHIP);

    if (state == RADIOLIB_ERR_NONE)
    {
        log_d("success!");
        state = radio.fixedPacketLengthMode(MSG_BUF_SIZE);
        if (state != RADIOLIB_ERR_NONE)
        {
            log_e("%s Error setting fixed packet length: [%d]", RECEIVER_CHIP, state);
            while (true)
                delay(10);
        }
        state = radio.setCrcFiltering(false);
        if (state != RADIOLIB_ERR_NONE)
        {
            log_e("%s Error disabling crc filtering: [%d]", RECEIVER_CHIP, state);
            while (true)
                delay(10);
        }
        uint8_t sync_word[] = {0xAA, 0x2D};
        state = radio.setSyncWord(sync_word, 2);
        if (state != RADIOLIB_ERR_NONE)
        {
            log_e("%s Error setting sync words: [%d]", RECEIVER_CHIP, state);
            while (true)
                delay(10);
        }
    }
        else
    {
        log_e("%s Error initialising: [%d]", RECEIVER_CHIP, state);
        while (true)
            delay(10);
    }
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    // Wait for time to be set
    if (!getLocalTime(&previous_time)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&previous_time, "%A, %B %d %Y %H:%M:%S");

    log_d("%s Setup complete - awaiting incoming messages...", RECEIVER_CHIP);
    float rssi = radio.getRSSI();

    // Set callback function
    radio.setPacketReceivedAction(setFlag);

    state = radio.startReceive();
    if (state != RADIOLIB_ERR_NONE)
    {
        log_e("%s startReceive() failed, code %d", RECEIVER_CHIP, state);
        while (true)
            delay(10);
    }
}


void loop() 
{   
    if (!mqtt_client.connected()) {
        setup_mqtt();
    }
    mqtt_client.loop();

    // Tries to receive radio message (non-blocking) and to decode it.
    // Timeout occurs after a small multiple of expected time-on-air.
    uint8_t recvData[MSG_BUF_SIZE];
    DecodeStatus decode_res = DECODE_INVALID;
        // Receive data
        if (receivedFlag)
        {
            receivedFlag = false;

            int state = radio.readData(recvData, MSG_BUF_SIZE);
            float rssi = radio.getRSSI();
            state = radio.startReceive();
            Serial.printf("RSSI: [%6.1fdBm]\n", rssi);

            if (state == RADIOLIB_ERR_NONE)
            {
                // Verify last syncword is 1st byte of payload (see setSyncWord() above)
                if (recvData[0] == 0xD4)
                {
                    log_d("%s R [%02X] RSSI: %0.1f", RECEIVER_CHIP, recvData[0], rssi);

                    decode_res = decoderPayload(&recvData[1], sizeof(recvData) - 1, &ws);
                    if (decode_res == DECODE_OK)
                    {
                        // Print decoded data
                    char batt_ok[] = "OK ";
                    char batt_low[] = "Low";
                    char batt_inv[] = "---";
                    char * batt;
                    if (ws.battery_ok) {
                        batt = batt_ok;
                    } else {
                        batt = batt_low;
                    }

                    //further processing of the data.
                    // calculate the time difference between the current and the previous data
                    struct tm current_time;
                    if (!getLocalTime(&current_time)) {
                        Serial.println("Failed to obtain time");
                        return;
                    }
                    else {
                        Serial.println(&current_time, "%A, %B %d %Y %H:%M:%S");
                        ws.timestamp = current_time;
                        ws.delta_t = difftime(mktime(&current_time), mktime(&previous_time));
                    }
                    
                    // calculate the rain difference
                    if (previous_rain <= 0.0) { // the first time we receive data
                        ws.delta_rain = -99.9;
                    }
                    else {
                        if  (ws.rain_mm < previous_rain) { // the rain gauge has been reset
                            previous_rain = 0.0;
                            ws.delta_rain = ws.rain_mm;
                        }
                        else {
                            ws.delta_rain = ws.rain_mm - previous_rain;
                            previous_rain = ws.rain_mm;
                        }
                    }
                
                    Serial.printf("Id: [%8X] Typ: [%X] Ch: [%d] St: [%d] Bat: [%-3s] RSSI: [%6.1fdBm] \n",
                        static_cast<int> (ws.sensor_id),
                        ws.s_type,
                        ws.chan,
                        ws.startup,
                        batt,
                        rssi
                    );
                
                        // Any other (weather-like) sensor is very similar
                    if (ws.temp_ok) {
                        Serial.printf("Temp: [%5.1fC] ", ws.temp_c);
                    } else {
                        Serial.printf("Temp: [---.-C] ");
                    }
                    if (ws.humidity_ok) {
                        Serial.printf("Hum: [%3d%%] ", ws.humidity);
                    }
                    else {
                        Serial.printf("Hum: [---%%] ");
                    }
                    if (ws.wind_ok) {
                        Serial.printf("Wmax: [%4.1fm/s] Wavg: [%4.1fm/s] Wdir: [%5.1fdeg] ",
                                ws.wind_gust_meter_sec,
                                ws.wind_avg_meter_sec,
                                ws.wind_direction_deg);
                    } else {
                        Serial.printf("Wmax: [--.-m/s] Wavg: [--.-m/s] Wdir: [---.-deg] ");
                    }
                    if (ws.rain_ok) {
                        Serial.printf("Total Rain: [%7.1fmm] ",  
                            ws.rain_mm);
                    } else {
                        Serial.printf("Total Rain: [-----.-mm] "); 
                    }
                    if (ws.uv_ok) {
                        Serial.printf("UVidx: [%2.1f] ",
                            ws.uv);
                    }
                    else {
                        Serial.printf("UVidx: [--.-] ");
                    }
                    if (ws.light_ok) {
                        Serial.printf("Light: [%2.1fklx] ",
                            ws.light_klx);
                    }
                    else {
                        Serial.printf("Light: [--.-klx] ");
                    }
                    Serial.printf("\n");

                    // Publish to MQTT and check result
                    // Publish to MQTT and check result
                    bool published = publishWeatherData(&ws, mqtt_client, mqtt_topic);
                    
                    if (published) {
                        log_d("Data published successfully to MQTT");
                        previous_time = current_time;
                        previous_rain = ws.rain_mm;
                    } else {
                        log_e("Failed to publish data to MQTT");
                        // Optionally retry or handle error
                    }


                    } // if (decode_res == DECODE_OK)
                    else
                    {
                        log_d("Decode failed: [%d] \n", decode_res);
                    }
                } // if (recvData[0] == 0xD4)


            } // if (state == RADIOLIB_ERR_NONE)
            else if (state == RADIOLIB_ERR_RX_TIMEOUT)
            {
                log_v("T");
            }
            else
            {
                // some other error occurred
                log_d("%s Receive failed: [%d]\n", RECEIVER_CHIP, state);
            }
        }

    delay(1000);
} // loop()
