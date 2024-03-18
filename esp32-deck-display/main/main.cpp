/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <cstdio>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "elm_display.hpp"
//todo watchdog
#define DISPLAY_UART (UART_NUM_1)
#define DISPLAY_TX_PIN (17)
static const char *TAG = "NMEA DISPLAY";

static const uint8_t SPLASH_PICTURE[] = {
#include "bitmaps/splash.txt"
};

#define BUF_SIZE (1024)
uint8_t brightness = 60; //todo 0
static uint8_t screen_buffer[SCREEN_WIDTH_BYTES * SCREEN_HEIGHT];

void displayCopyPict(const uint8_t *background) {
    memcpy(screen_buffer, background, SCREEN_WIDTH_BYTES * SCREEN_HEIGHT);
}

void displayPaint() {
    uart_write_bytes(DISPLAY_UART, "\xAA\xA0", 2);
    uart_write_bytes(DISPLAY_UART, &brightness, 1);
    for (int y = 127; y >= 0; y--) {
        for (int i = 0; i < 30; i++) {
            uint8_t byte = screen_buffer[y * 30 + i];
            uart_write_bytes(DISPLAY_UART, &byte, 1);
            if (byte == 0xAA) {//escaping
                uart_write_bytes(DISPLAY_UART, "\xA9", 1);
            }
        }
    }
}

void splashLcd() {

    auto xLastWakeTime = xTaskGetTickCount();

    for (int i = 0; i < (2 * SCREEN_WIDTH_BYTES) + 1; i += 2) {
        displayCopyPict(SPLASH_PICTURE);
        for (int j = 0; j < SCREEN_HEIGHT; j++) {
            int k = -j / 5 + i;
            if (k < 0) k = 0;
            for (; k < SCREEN_WIDTH_BYTES; k++) {
                screen_buffer[j * SCREEN_WIDTH_BYTES + k] = 0;
            }
        }
        displayPaint();
    }
    vTaskDelayUntil(&xLastWakeTime, 70 * xPortGetTickRateHz() / 1000);
}

[[noreturn]] static void display_task([[maybe_unused]] void *arg) {
    uart_config_t uart_config = {
            .baud_rate = 625000,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 16,
            .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(DISPLAY_UART, BUF_SIZE * 2, 0, 0, nullptr, 0));
    ESP_ERROR_CHECK(uart_param_config(DISPLAY_UART, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(DISPLAY_UART, DISPLAY_TX_PIN, -1, -1, -1));

    while (true) {
        splashLcd();
        auto xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 1500 * xPortGetTickRateHz() / 1000);
    }
}

extern "C" [[maybe_unused]] void app_main() {
    xTaskCreate(display_task, "uart_echo_task", 2048, nullptr, 10, nullptr);
}
