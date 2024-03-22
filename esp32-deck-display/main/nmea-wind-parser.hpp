//
// Created by elmot on 20 Mar 2024.
//

#ifndef TEST_NMEA_WIND_PARSER_HPP
#define TEST_NMEA_WIND_PARSER_HPP

#ifdef NMEA_WIND_PARSER_TEST
typedef int TickType_t ;
extern TickType_t xTaskGetTickCount();
#else
#include "freertos/FreeRTOS.h"
#endif

typedef enum  {
    ANEMOMETER_EXPECT_WIFI,
    ANEMOMETER_OK,
    ANEMOMETER_DATA_FAIL,
    ANEMOMETER_CONN_TIMEOUT,
    ANEMOMETER_CONN_FAIL
} anemometer_state_t;

typedef struct {
    volatile anemometer_state_t state;
    volatile int windAngle;
    volatile float windSpdMps;
    volatile int backLightPercent;
    volatile bool angleAlarm;
    volatile TickType_t timestamp;
} wind_data_t;

extern volatile wind_data_t windData;

bool parseNmea(const char *nmeaString);

#endif //TEST_NMEA_WIND_PARSER_HPP
