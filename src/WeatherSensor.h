#ifndef WEATHERSENSOR_H
#define WEATHERSENSOR_H

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Weather Data structure for storing sensor measurements
 * Based on the Bresser 7-in-1 weather sensor implementation by Matthias Prinke.
 */
struct WeatherData {
            struct tm  timestamp;           //!< timestamp
            uint32_t sensor_id;          //!< sensor ID
            float    temp_c;            //!< temperature in degC
            float    light_klx;         //!< Light KLux (only 7-in-1)
            float    light_lux;         //!< Light lux (only 7-in-1)
            float    uv;                //!< uv radiation (only 6-in-1 & 7-in-1)
            float    rain_mm;           //!< rain gauge level in mm
            float    wind_direction_deg;  //!< wind direction in deg
            float    wind_gust_meter_sec; //!< wind speed (gusts) in m/s
            float    wind_avg_meter_sec;  //!< wind speed (avg)   in m/s
            float    delta_t;           //!< time difference in seconds
            float    delta_rain;        //!< rain difference in mm
            uint8_t  humidity;                //!< humidity in %
            bool     temp_ok;         //!< temperature o.k. (only 6-in-1)
            bool     humidity_ok;     //!< humidity o.k.
            bool     light_ok;        //!< light o.k. (only 7-in-1)
            bool     uv_ok;           //!< uv radiation o.k. (only 6-in-1)
            bool     wind_ok;         //!< wind speed/direction o.k. (only 6-in-1)
            bool     rain_ok;         //!< rain gauge level o.k.
            uint8_t  s_type;          //!< sensor type
            uint8_t  chan;            //!< channel
            bool     battery_ok;      //!< battery status
            bool     valid;           //!< valid data
            bool     complete;        //!< complete data
            bool     startup;         //!< startup flag
};
typedef struct WeatherData weather_data_t;

struct Sensor
{   
    uint32_t sensor_id;     //!< sensor ID
    uint8_t  s_type;        //!< sensor type
    uint8_t  chan;          //!< channel
    uint8_t  decoder;       //!< decoder type
    int      rssi;          //!< RSSI
    bool     battery_ok;    //!< battery status
    bool     valid;         //!< valid data
    bool     complete;      //!< complete data
    bool     startup;       //!< startup flag
    weather_data_t w;          //!< weather data
};

typedef struct Sensor sensor_t;
// Function prototypes go here
// Add them as you implement functions in the .c file

#endif /* WEATHERSENSOR_H */