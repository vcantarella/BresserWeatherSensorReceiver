#include <esp_sleep.h>
#include <time.h>

// RTC memory
RTC_DATA_ATTR time_t lastWakeTime;

void setup() {
    Serial.begin(115200);

    // Check if the ESP32 woke up from deep sleep
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Woke up from deep sleep");
        Serial.print("Last wake time: ");
        Serial.println(ctime(&lastWakeTime));
    } else {
        Serial.println("Normal boot");
    }

    // Get the current time
    time(&lastWakeTime);
    Serial.print("Current time: ");
    Serial.println(ctime(&lastWakeTime));

    // Set the wakeup timer
    esp_sleep_enable_timer_wakeup(10 * 1000000); // 10 seconds
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();
}

void loop() {
    // This will never be called
}