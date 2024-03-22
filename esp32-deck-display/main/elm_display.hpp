//
// Created by elmot on 18 Mar 2024.
//
#ifndef ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H
#define ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H
#include "nmea-wind-parser.hpp"

#define SCREEN_WIDTH (240)
#define SCREEN_WIDTH_BYTES (SCREEN_WIDTH / 8)
#define SCREEN_HEIGHT   (128)

#define SCREEN_UPDATE_TIMEOUT_MS (70)
#define INCOMING_DATA_TIMEOUT (3000)

void lcdInit();

void lcdSplash();

void lcdMainScreenUpdatePicture();

extern SemaphoreHandle_t updateSemaphore;

extern volatile uint8_t lcdBrightness;

#endif //ESP32_WINDSENSOR_DECK_ELM_DISPLAY_H
