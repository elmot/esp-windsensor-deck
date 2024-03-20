//
// Created by elmot on 18 Mar 2024.
//
#ifndef ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H
#define ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H
#include "nmea-wind-parser.hpp"

#define SCREEN_WIDTH (240)
#define SCREEN_WIDTH_BYTES (SCREEN_WIDTH / 8)
#define SCREEN_HEIGHT   (128)

void lcdInit(void);

void lcdSplash(void);

void lcdMainScreenUpdatePicture(void);

#endif //ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H
