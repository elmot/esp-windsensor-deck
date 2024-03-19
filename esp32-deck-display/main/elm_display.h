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
    OK,
    CONN_TIMEOUT,
    CONN_FAIL
};

struct wind_state {
    enum anem_state anemState;
    int windAngle;
    float windSpdMps;
    int backLightPercent;
    bool angleAlarm;
};

extern struct wind_state state;

void lcdInit(void);

void lcdSplash(void);

void lcdMainScreenUpdatePicture(void);

#ifdef __cplusplus
};
#endif

#endif //ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H
