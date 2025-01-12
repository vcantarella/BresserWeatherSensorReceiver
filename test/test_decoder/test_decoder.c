#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../../src/Decoder.h"
#include "../../src/WeatherSensor.h"

// Very small test helpers
int failed = 0;
#define TEST(name) void name()
#define RUN_TEST(name) printf("\n\033[1m%s\n\033[0m", #name); name()
#define ASSERT(expr) if (!(expr)) { \
  failed = 1; \
  printf("\033[0;31mFAIL: %s\n\033[0m", #expr); \
} else { \
  printf("\033[0;32mPASS: %s\n\033[0m", #expr); \
}
#define ASSERT_STR_EQ(str1, str2) ASSERT(strcmp(str1, str2) == 0)
// End of test helpers

int main(){
// Test data
const uint8_t msg[] = {0xC4, 0xD6, 0x3A, 0xC5, 0xBD, 0xFA, 0x18, 0xAA, 0xAA, 0xAA, 0xAA, 0xAB, 0xFC, 0xAA, 0x98, 0xDA, 0x89, 0xA3, 0x2F,
     0xEC, 0xAF, 0x9A, 0xAA, 0xAA, 0xAA, 0x00};

uint8_t msg_size = sizeof(msg) / sizeof(msg[0]);

weather_data_t ws;

DecodeStatus status = decoderPayload(msg, msg_size, &ws);

if (ws.temp_ok == true) {
    printf("Temp: [%5.1fC] ", ws.temp_c);
} else {
    printf("Temp: [---.-C] ");
}
if (ws.humidity_ok == true) {
    printf("Hum: [%3d%%] ", ws.humidity);
}
else {
    printf("Hum: [---%%] ");
}
if (ws.wind_ok == true) {
    printf("Wmax: [%4.1fm/s] Wavg: [%4.1fm/s] Wdir: [%5.1fdeg] ",
            ws.wind_gust_meter_sec,
            ws.wind_avg_meter_sec,
            ws.wind_direction_deg);
} else {
    printf("Wmax: [--.-m/s] Wavg: [--.-m/s] Wdir: [---.-deg] ");
}
if (ws.rain_ok == true) {
    printf("Rain: [%7.1fmm] ",  
        ws.rain_mm);
} else {
    printf("Rain: [-----.-mm] "); 
}
if (ws.uv_ok == true) {
    printf("UVidx: [%1.1f] ",
        ws.uv);
}
else {
    printf("UVidx: [-.-%%] ");
}
if (ws.light_ok == true) {
    printf("Light: [%2.1fKlux] ",
        ws.light_klx);
}
else {
    printf("Light: [--.-Klux] ");
};
}
// expected print: Id: [    906F] Typ: [1] Ch: [0] St: [0] Bat: [OK ] RSSI: [ -56.0dBm] Temp: [ 32.7C] Hum: [ 23%] Wmax: [ 0.0m/s] Wavg: [ 0.0m/s] Wdir: [175.0deg] Rain: [   15.6mm] UVidx: [5.3] Light: [98.5Klux] 
// // Test for sanity check failure
// TEST(test_sanity_check_failure) {
//     printf("Executing test_sanity_check_failure...\n"); fflush(stdout);
//     uint8_t msg[MSG_BUF_SIZE] = {0};
//     msg[21] = 0x00; // Trigger sanity check failure
//     WeatherData wd;
//     DecodeStatus status = decodeBresser7In1Payload(msg, MSG_BUF_SIZE, &wd);
//     ASSERT(status == DECODE_PAR_ERR);
// }

// // Test for checksum verification failure
// TEST(test_checksum_verification_failure) {
//     printf("Executing test_checksum_verification_failure...\n"); fflush(stdout);
//     uint8_t msg[MSG_BUF_SIZE] = {0};
//     msg[21] = 0x01; // Pass sanity check
//     msg[0] = 0x12; // Mock checksum bytes
//     msg[1] = 0x34;
//     WeatherData wd;
//     DecodeStatus status = decodeBresser7In1Payload(msg, MSG_BUF_SIZE, &wd);
//     ASSERT(status == DECODE_CHK_ERR);
// }

// // Test for successful decoding
// TEST(test_successful_decoding) {
//     printf("Executing test_successful_decoding...\n"); fflush(stdout);
//     uint8_t msg[MSG_BUF_SIZE] = {0};
//     msg[21] = 0x01; // Pass sanity check
//     msg[0] = 0x12; // Mock checksum bytes
//     msg[1] = 0x34;
//     // Mock valid message that passes checksum verification
//     WeatherData wd;
//     DecodeStatus status = decodeBresser7In1Payload(msg, MSG_BUF_SIZE, &wd);
//     printf("Decode status: %d\n", status);
//     ASSERT(status == DECODE_OK);
// }

// // Test function
// void test_decodeBresser7In1Payload() {
//     uint8_t msg[MSG_BUF_SIZE] = {
//         0x12, 0x34, // Mock checksum bytes
//         0x00, 0x00, 0x00, 0x00, // Padding bytes
//         0x00, 0x00, 0x00, 0x00, // Padding bytes
//         0x00, 0x00, 0x00, 0x00, // Padding bytes
//         0x00, 0x00, 0x00, 0x00, // Padding bytes
//         0x00, 0x00, 0x00, 0x01  // Pass sanity check
//     };

//     // Fill in the rest of the message with meaningful data
//     // Example: msg[4] to msg[21] should be filled with data that will be decoded into WeatherData

//     WeatherData wd;
//     DecodeStatus status = decodeBresser7In1Payload(msg, MSG_BUF_SIZE, &wd);
//     //assert(status == DECODE_OK);

//     // Print the decoded data for verification
//     printf("Weather Data:\n");
//     printf("Wind Direction (deg): %f\n", wd.wind_direction_deg);
//     printf("Wind Gust (m/s): %f\n", wd.wind_gust_meter_sec);
//     printf("Wind Average (m/s): %f\n", wd.wind_avg_meter_sec);
//     printf("Rain (mm): %f\n", wd.rain_mm);
//     printf("Temperature (C): %f\n", wd.temp_c);
//     printf("Humidity: %d\n", wd.humidity);
//     //printf("Light Raw: %d\n", wd.lght_raw);
//     //printf("UV Raw: %d\n", wd.uv_raw);
// }


// int main() {
//     RUN_TEST(test_sanity_check_failure);
//     RUN_TEST(test_checksum_verification_failure);

//     test_decodeBresser7In1Payload();

//     RUN_TEST(test_successful_decoding);

//     if (failed) {
//         printf("\n\033[0;31mSome tests failed.\n\033[0m");
//     } else {
//         printf("\n\033[0;32mAll tests passed.\n\033[0m");
//     }

//     return 0;
// }