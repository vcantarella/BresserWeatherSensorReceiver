///////////////////////////////////////////////////////////////////////////////////////////////////
// WeatherSensorCfg.h
//
// Bresser 5-in-1/6-in-1/7-in-1 868 MHz Weather Sensor Radio Receiver
// based on CC1101, SX1276/RFM95W, SX1262 or LR1121 and ESP32/ESP8266
//
// https://github.com/matthias-bs/BresserWeatherSensorReceiver
//
// NOTE: Application/hardware specific configurations should be made in this file!
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
// 20230124 Added some default settings based on selected boards in Arduino IDE
// 20230207 Added pin definitions for ARDUINO_TTGO_LoRa32_v21new
// 20230208 Added pin definitions for ARDUINO_TTGO_LoRa32_V2
// 20230301 Added pin definitions for Wireless_Stick (from Heltec)
// 20230316 Added pin definitions for Adafruit Feather ESP32 with RFM95W "FeatherWing" ADA3232
// 20230330 Added pin definitions and changes for Adafruit Feather 32u4 (AVR) RFM95 LoRa Radio 
// 20230412 Added workaround for Professional Wind Gauge / Anemometer, P/N 7002531
// 20230420 Added pin definitions for DFRobot FireBeetle ESP32 with FireBeetle Cover LoRa
// 20230607 Added pin definitions for Heltec WiFi LoRa 32(V2)
// 20230624 Added Bresser Lightning Sensor decoder
// 20230804 Added Bresser Water Leakage Sensor decoder
// 20230926 Added pin definitions for Adafruit Feather RP2040 with RFM95W "FeatherWing" ADA3232
// 20230927 Removed _DEBUG_MODE_ (log_d() is used instead)
// 20231004 Added function names and line numbers to ESP8266/RP2040 debug logging
// 20231101 Added USE_SX1262 for Heltec Wireless Stick V3
// 20231121 Added Heltec WiFi LoRa32 V3
// 20231130 Bresser 3-in-1 Professional Wind Gauge / Anemometer, PN 7002531: Replaced workaround 
//          for negative temperatures by fix (6-in-1 decoder)
// 20231227 Added RAINGAUGE_USE_PREFS and LIGHTNING_USE_PREFS
// 20240122 Modified for unit testing
// 20240204 Added separate ARDUINO_heltec_wireless_stick_v2/v3
// 20240322 Added pin definitions for M5Stack Core2 with M5Stack Module LoRa868
// 20240415 Added pin definitions for ESP32-S3 PowerFeather with with RFM95W "FeatherWing" ADA3232
// 20240417 Modified SENSOR_IDS_INC
// 20240425 Added define variant ARDUINO_heltec_wifi_lora_32_V3
// 20240507 Renamed NUM_SENSORS to MAX_SENSORS_DEFAULT
//          NOTE: ARDUINO_ARCH_AVR no longer supported due to code size!!!
// 20240508 Updated board definitions after changes in arduino-esp32 v3.0.0
// 20240509 Fixed ARDUINO_HELTEC_WIRELESS_STICK_V3
// 20240904 Added ARDUINO_ESP32S3_DEV
// 20240910 Heltec: Fixed pin definitions
// 20241030 Added pin definitions for Maker Go ESP32C3 Supermini with Heltec HT-RA62
// 20241130 Added pin definitions for Heltec Vision Master T190
// 20241205 Added pin definitions for Lilygo T3-S3 (SX1262/SX1276/LR1121)
// 20241227 Improved maintainability of board definitions
//
// ToDo:
// -
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WEATHER_SENSOR_CFG_H
#define WEATHER_SENSOR_CFG_H

#include <Arduino.h>

// ------------------------------------------------------------------------------------------------
// --- Weather Sensors ---
// ------------------------------------------------------------------------------------------------
#define MAX_SENSORS_DEFAULT 1       // Maximum number of sensors to be received

// List of sensor IDs to be excluded - can be empty
#define SENSOR_IDS_EXC { }

// List of sensor IDs to be included - if empty, handle all available sensors
#define SENSOR_IDS_INC { }

// Maximum number of sensor IDs in include/exclude list
#define MAX_SENSOR_IDS 12

// Disable data type which will not be used to save RAM
#define WIND_DATA_FLOATINGPOINT
#define WIND_DATA_FIXEDPOINT

// Select appropriate sensor message format(s)
// Comment out unused decoders to save operation time/power
#define BRESSER_7_IN_1


// ------------------------------------------------------------------------------------------------
// --- Rain Gauge / Lightning sensor data retention during deep sleep ---
// ------------------------------------------------------------------------------------------------

#if !defined(INSIDE_UNITTEST)
    #if defined(ESP32)
        // Option: Comment out to save data in RTC RAM
        // N.B.:
        // ESP8266 has RTC RAM, too, but access is different from ESP32
        // and currently not implemented here
        #define RAINGAUGE_USE_PREFS
        #define LIGHTNING_USE_PREFS
    #else
        // Using Preferences is mandatory on other architectures (e.g. RP2040)
        #define RAINGAUGE_USE_PREFS
        #define LIGHTNING_USE_PREFS
    #endif
#endif

// ------------------------------------------------------------------------------------------------
// --- Board ---
// ------------------------------------------------------------------------------------------------

// LILYGO TTGO LoRaP32 board with integrated RF tranceiver (SX1276)
// See pin definitions in
// https://github.com/espressif/arduino-esp32/tree/master/variants/ttgo-lora32-*
// and
// https://www.thethingsnetwork.org/forum/t/big-esp32-sx127x-topic-part-2/11973

// This define is set by selecting "Board: TTGO LoRa32-OLED" / "Board Revision: TTGO LoRa32 V1 (No TFCard)"
// in the Arduino IDE:
//#define ARDUINO_TTGO_LoRa32_V1

// This define is set by selecting "Board: TTGO LoRa32-OLED" / "Board Revision: TTGO LoRa32 V2"
// in the Arduino IDE:
//#define ARDUINO_TTGO_LoRa32_V2

// This define is set by selecting "Board: TTGO LoRa32-OLED" / "Board Revision: TTGO LoRa32 V2.1 (1.6.1)"
// in the Arduino IDE:
//#define ARDUINO_TTGO_LoRa32_V21new

#pragma message("ARDUINO_TTGO_LoRa32_V21new defined; using on-board transceiver")
#define USE_SX1276
// Use pinning for LILIGO TTGO LoRa32-OLED V2.1 (1.6.1)
#define PIN_RECEIVER_CS   LORA_CS
#define PIN_RECEIVER_IRQ  LORA_IRQ
#define PIN_RECEIVER_GPIO LORA_D1
#define PIN_RECEIVER_RST  LORA_RST
#define RADIO_CHIP SX1276

// #elif defined(ESP32)
//     #pragma message("ESP32 defined; this is a generic (i.e. non-specific) target")
//     #pragma message("Cross check if the selected GPIO pins are really available on your board.")
//     #pragma message("Connect a radio module with a supported chip.")
//     #pragma message("Select the chip by setting the appropriate define.")
//     #define USE_SX1276
//     //#define USE_SX1262
//     //#define USE_CC1101
//     //#define USE_LR1121
//     // Generic pinning for ESP32 development boards
//     #define PIN_RECEIVER_CS   27
//     #define PIN_RECEIVER_IRQ  21
//     #define PIN_RECEIVER_GPIO 33
//     #define PIN_RECEIVER_RST  32
// #elif defined(ESP8266)
//     #pragma message("ESP8266 defined; this is a generic (i.e. non-specific) target")
//     #pragma message("Cross check if the selected GPIO pins are really available on your board.")
//     #pragma message("Connect a radio module with a supported chip.")
//     #pragma message("Select the chip by setting the appropriate define.")
//     //#define USE_SX1276
//     //#define USE_SX1262
//     #define USE_CC1101
//     //#define USE_LR1121

//     // Generic pinning for ESP8266 development boards (e.g. LOLIN/WEMOS D1 mini)
//     #define PIN_RECEIVER_CS   15
//     #define PIN_RECEIVER_IRQ  4
//     #define PIN_RECEIVER_GPIO 5
//     #define PIN_RECEIVER_RST  2
// #endif

// ------------------------------------------------------------------------------------------------
// --- Debug Logging Output ---
// ------------------------------------------------------------------------------------------------
// - ESP32:
//   CORE_DEBUG_LEVEL is set in Adruino IDE:
//   Tools->Core Debug Level: "<None>|<Error>|<Warning>|<Info>|<Debug>|<Verbose>"
//   https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-log.h
//
// - ESP8266:
//   DEBUG_ESP_PORT is set in Arduino IDE:
//   Tools->Debug port: "<None>|<Serial>|<Serial1>"
//
// - RP2040:
//   DEBUG_RP2040_PORT is set in Arduino IDE:
//   Tools->Debug port: "<Disabled>|<Serial>|<Serial1>|<Serial2>"
//
//   Replacement for
//   https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-log.h
//   on ESP8266 and RP2040:

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define RECEIVER_CHIP "[" STR(RADIO_CHIP) "]"
#pragma message("Receiver chip: " RECEIVER_CHIP)
#pragma message("Pin config: RST->" STR(PIN_RECEIVER_RST) ", CS->" STR(PIN_RECEIVER_CS) ", GD0/G0/IRQ->" STR(PIN_RECEIVER_IRQ) ", GDO2/G1/GPIO->" STR(PIN_RECEIVER_GPIO) )

#endif
