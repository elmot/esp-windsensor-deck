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

enum anem_state {
    ANEMOMETER_OK,
    ANEMOMETER_DATA_FAIL,
    ANEMOMETER_CONN_TIMEOUT,
    ANEMOMETER_CONN_FAIL
};

struct wind_state {
    volatile enum anem_state anemState;
    volatile int windAngle;
    volatile float windSpdMps;
    volatile int backLightPercent;
    volatile bool angleAlarm;
    volatile TickType_t timestamp;
};

extern wind_state state;

bool parseNmea(const char *nmeaString);

#endif //TEST_NMEA_WIND_PARSER_HPP
