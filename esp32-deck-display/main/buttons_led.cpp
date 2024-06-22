#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "elm_display.hpp"
#include "driver/gpio.h"

void __attribute__((noreturn)) buttons_task(void *arg) {
    ESP_UNUSED(arg);
    static const gpio_config_t gpioConfig = {
            .pin_bit_mask = BIT(CONFIG_BACKLIGHT_DOWN_PIN) | BIT(CONFIG_BACKLIGHT_UP_PIN),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
    };
    static const uint8_t brightness[7] = {0, 5, 10, 20, 40, 70, 100};
    static int brightnessIdx = 0;
    static int cycleIndex = 0;
    gpio_config(&gpioConfig);
    while (true) {
        bool up = gpio_get_level((gpio_num_t) CONFIG_BACKLIGHT_UP_PIN) == 0;
        bool down = gpio_get_level((gpio_num_t) CONFIG_BACKLIGHT_DOWN_PIN) == 0;
        if (up == down) {
            cycleIndex = 0;
        } else if (up && brightnessIdx == 0 && cycleIndex > 7) {
            brightnessIdx = 1;
            cycleIndex = 0;
            xSemaphoreGive(updateSemaphore);
        } else if (cycleIndex > 2) {
            if (up) {
                brightnessIdx = MIN(sizeof brightness - 1, brightnessIdx + 1);
            } else {
                brightnessIdx = MAX(0, brightnessIdx - 1);
            }
            lcdBrightness = brightness[brightnessIdx];
            cycleIndex = 0;
            xSemaphoreGive(updateSemaphore);
        } else {
            cycleIndex++;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

}

void __attribute__((noreturn)) alarm_led_task(void *args) {
    ESP_UNUSED(args);
    static const gpio_config_t gpioConfig = {
            .pin_bit_mask = BIT(CONFIG_ALARM_LED_PIN) | BIT(CONFIG_ALARM_LED_PIN_INVERTED),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&gpioConfig);
    for (int level = 0; true;) {
        TickType_t delay;
        switch (windData.state) {
            case ANEMOMETER_EXPECT_WIFI:
                delay = 1500;
                break;
            case ANEMOMETER_OK:
                delay = 600;
                if (!windData.angleAlarm) {
                    level = 0;
                }
                break;
            case ANEMOMETER_DATA_FAIL:
                delay = 100;
                break;
            default:
                delay = 200;
        }
        gpio_set_level((gpio_num_t) CONFIG_ALARM_LED_PIN, level);
        gpio_set_level((gpio_num_t) CONFIG_ALARM_LED_PIN_INVERTED, !level);
        xSemaphoreTake(alarmSemaphore, pdMS_TO_TICKS(delay));
        level = !level;
    }
}


