//
// Created by elmot on 18 Mar 2024.
//

#ifdef __cplusplus
extern "C" {
#endif
#ifndef ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H
#define ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H


#define SCREEN_WIDTH (240)
#define SCREEN_WIDTH_BYTES (SCREEN_WIDTH / 8)
#define SCREEN_HEIGHT   (128)

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

extern struct wind_state state;

void lcdInit(void);

void lcdSplash(void);

void lcdMainScreenUpdatePicture(void);

bool scanAlarm(const char *nmeaExpr);
bool scanWind(const char *nmeaExpr);

#ifdef __cplusplus
};
#endif

#endif //ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H
