#include <WiFi.h>
#include <time.h>

// WiFi credentials
const char* ssid = "Vodafone-8D91";
const char* password = "FRHUGCPHAchbhCQc";

// NTP server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize and configure NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Wait for time to be set
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void loop() {
    // Print the current time every second
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    }
    delay(1000);
}