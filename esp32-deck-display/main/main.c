#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "elm_display.h"
//todo watchdog
static const char *TAG = "NMEA DISPLAY";

struct wind_state state = {
        .backLightPercent =50,
        .anemState=OK,
        .angleAlarm=true,
        .windAngle=27,
        .windSpdMps=6};

static void __attribute__((noreturn)) display_task([[maybe_unused]] void *arg) {
    ESP_UNUSED(TAG);
    ESP_UNUSED(arg);
    lcdInit();
    lcdSplash();
    while (true) {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 1500 * xPortGetTickRateHz() / 1000);
        lcdMainScreenUpdatePicture();

            { //todo remove
            state.windAngle+=1;
            state.windSpdMps+=1;
            state.angleAlarm =! state.angleAlarm;
            state.anemState = (state.anemState+1)%3;
            }

    }
}

void app_main() {
     ESP_UNUSED(app_main);
     xTaskCreate(display_task, "uart_echo_task", 2048, NULL, 10, NULL);
}

