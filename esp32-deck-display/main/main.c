/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "elm_display.h"
//todo watchdog
static const char *TAG = "NMEA DISPLAY";

[[noreturn]] static void display_task([[maybe_unused]] void *arg) {
ESP_UNUSED(arg);
    lcdInit();
    while (true) {
        lcdSplash();
        TickType_t xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 1500 * xPortGetTickRateHz() / 1000);
    }
}
 void app_main() {
     ESP_UNUSED(app_main);
     xTaskCreate(display_task, "uart_echo_task", 2048, NULL, 10, NULL);
}
