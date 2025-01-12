#include "WeatherSensor.h"
#include "Decoder.h"
#include <stdio.h>
// #include "WeatherSensorCfg.h"

// Radio message decoding status
// typedef enum DecodeStatus {
//     DECODE_INVALID, DECODE_OK, DECODE_PAR_ERR, DECODE_CHK_ERR, DECODE_DIG_ERR, DECODE_SKIP, DECODE_FULL
// } DecodeStatus;

// uint8_t MSG_BUF_SIZE = 27;

//
// From Mathias Prinke's Bresser 7-in-1 weather sensor implementation
// From from rtl_433 project - https://github.com/merbanan/rtl_433/blob/master/src/util.c
//
uint16_t lfsr_digest16(uint8_t const message[], unsigned bytes, uint16_t gen, uint16_t key)
{
    uint16_t sum = 0;
    for (unsigned k = 0; k < bytes; ++k)
    {
        uint8_t data = message[k];
        for (int i = 7; i >= 0; --i)
        {
            // fprintf(stderr, "key at bit %d : %04x\n", i, key);
            // if data bit is set then xor with key
            if ((data >> i) & 1)
                sum ^= key;

            // roll the key right (actually the lsb is dropped here)
            // and apply the gen (needs to include the dropped lsb as msb)
            if (key & 1)
                key = (key >> 1) ^ gen;
            else
                key = (key >> 1);
        }
    }
    return sum;
}

DecodeStatus decoderPayload(uint8_t const *msg, uint8_t msgSize,
   weather_data_t *ws)
   {
        if (msg[21] == 0x00)
        {
            printf("Data sanity check failed");
            return DECODE_INVALID;
        }
    
        // data de-whitening
        uint8_t msgw[MSG_BUF_SIZE];
        for (unsigned i = 0; i < msgSize; ++i)
        {
            msgw[i] = msg[i] ^ 0xaa;
        }
        // #if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
        //     printf("De-whitened Data [%04X], [%02X]\n", msgw, msgSize);
        // #endif
    
        // LFSR-16 digest, generator 0x8810 key 0xba95 final xor 0x6df1
        int chkdgst = (msgw[0] << 8) | msgw[1];
        int digest = lfsr_digest16(&msgw[2], 23, 0x8810, 0xba95); // bresser_7in1
        if ((chkdgst ^ digest) != 0x6df1)
        { // bresser_7in1
            printf("Digest check failed - [%04X] vs [%04X] (%04X)\n", chkdgst, digest, chkdgst ^ digest);
            return DECODE_DIG_ERR;
        }

        int id_tmp = (msgw[2] << 8) | (msgw[3]);
        int s_type = msg[6] >> 4; // raw data, no de-whitening

        int flags = (msgw[15] & 0x0f);
        int battery_low = (flags & 0x06) == 0x06;

        ws->sensor_id = id_tmp;
        ws->s_type = s_type;
        ws->startup = (msgw[6] & 0x08) == 0x00; // raw data, no de-whitening
        ws->chan = msgw[6] & 0x07;              // raw data, no de-whitening
        ws->battery_ok = !battery_low;
        ws->valid = true;
        ws->complete = true;

    int wdir = (msgw[4] >> 4) * 100 + (msgw[4] & 0x0f) * 10 + (msgw[5] >> 4);
    int wgst_raw = (msgw[7] >> 4) * 100 + (msgw[7] & 0x0f) * 10 + (msgw[8] >> 4);
    int wavg_raw = (msgw[8] & 0x0f) * 100 + (msgw[9] >> 4) * 10 + (msgw[9] & 0x0f);
    int rain_raw = (msgw[10] >> 4) * 100000 + (msgw[10] & 0x0f) * 10000 + (msgw[11] >> 4) * 1000 + (msgw[11] & 0x0f) * 100 + (msgw[12] >> 4) * 10 + (msgw[12] & 0x0f) * 1; // 6 digits
    float wgst = wgst_raw * 0.1f;
    float wavg = wavg_raw * 0.1f;
    float wdir_deg = wdir * 1.0f;
    float rain_mm = rain_raw * 0.1f;
    int temp_raw = (msgw[14] >> 4) * 100 + (msgw[14] & 0x0f) * 10 + (msgw[15] >> 4);
    float temp_c = temp_raw * 0.1f;
    if (temp_raw > 600)
        temp_c = (temp_raw - 1000) * 0.1f;
    int humidity = (msgw[16] >> 4) * 10 + (msgw[16] & 0x0f);
    int lght_raw = (msgw[17] >> 4) * 100000 + (msgw[17] & 0x0f) * 10000 + (msgw[18] >> 4) * 1000 + (msgw[18] & 0x0f) * 100 + (msgw[19] >> 4) * 10 + (msgw[19] & 0x0f);
    int uv_raw = (msgw[20] >> 4) * 100 + (msgw[20] & 0x0f) * 10 + (msgw[21] >> 4);

    float light_klx = lght_raw * 0.001f; // TODO: remove this
    float light_lux = lght_raw;
    float uv_index = uv_raw * 0.1f;

    ws->temp_ok = true;
    ws->humidity_ok = true;
    ws->wind_ok = true;
    ws->rain_ok = true;
    ws->light_ok = true;
    ws->uv_ok = true;
    ws->temp_c = temp_c;
    ws->humidity = humidity;
    ws->wind_gust_meter_sec = wgst;
    ws->wind_avg_meter_sec = wavg;
    ws->wind_direction_deg = wdir_deg;
    ws->rain_mm = rain_mm;
    ws->light_klx = light_klx;
    ws->light_lux = light_lux;
    ws->uv = uv_index;

    return DECODE_OK;
   }