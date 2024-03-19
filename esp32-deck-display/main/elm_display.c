#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "elm_display.h"
uint8_t brightness = 60; //todo 0

#define BUF_SIZE (1024)
#define DISPLAY_UART (UART_NUM_1)
#define DISPLAY_TX_PIN (17)


static const uint8_t SPLASH_PICTURE[] = {
#include "bitmaps/splash.txt"
};

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

void lcdInit() {
    uart_config_t uart_config = {
            .baud_rate = 625000,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 16,
            .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_driver_install(DISPLAY_UART, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(DISPLAY_UART, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(DISPLAY_UART, DISPLAY_TX_PIN, -1, -1, -1));

}
void lcdSplash() {

    TickType_t xLastWakeTime = xTaskGetTickCount();

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
