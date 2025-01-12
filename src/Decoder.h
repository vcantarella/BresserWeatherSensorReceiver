#ifndef DECODER_H
#define DECODER_H

#include "WeatherSensor.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Radio message decoding status
 */
typedef enum DecodeStatus {
    DECODE_INVALID,  //!< Invalid decode
    DECODE_OK,       //!< Decode successful
    DECODE_PAR_ERR,  //!< Parameter error
    DECODE_CHK_ERR,  //!< Checksum error
    DECODE_DIG_ERR,  //!< Digital error
    DECODE_SKIP,     //!< Skip decode
    DECODE_FULL      //!< Buffer full
} DecodeStatus;

#define MSG_BUF_SIZE 27

/**
 * @brief Decode weather sensor payload
 * @param msg Input message buffer containing sensor data
 * @param msgSize Length of the message buffer
 * @param ws Pointer to WeatherData structure to store decoded data
 * @return DecodeStatus indicating success or failure of decoding
 */
DecodeStatus decoderPayload(uint8_t const *msg, uint8_t msgSize,
   weather_data_t *ws);

#ifdef __cplusplus
}
#endif

#endif /* DECODER_H */