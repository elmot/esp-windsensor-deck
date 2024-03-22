#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <esp_wifi.h>

#include "elm_display.hpp"

//todo watchdog
//todo brightness buttons
//todo backlight
static const char *TAG = "NMEA DISPLAY";

volatile wind_data_t windData = {
        .state = ANEMOMETER_EXPECT_WIFI,
        .windAngle = 27,
        .windSpdMps = 6,
        .angleAlarm = false,
        .timestamp = 0};

static void __attribute__((noreturn)) display_task(void *arg) {
    ESP_UNUSED(TAG);
    ESP_UNUSED(arg);
    lcdInit();
    lcdSplash();
    while (true) {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        xSemaphoreTake(updateSemaphore, pdMS_TO_TICKS(1000));
        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SCREEN_UPDATE_TIMEOUT_MS));
        lcdMainScreenUpdatePicture();
    }
}

static void udp_listener_task(void *arg);

SemaphoreHandle_t updateSemaphore;
static StaticSemaphore_t updateSemaphoreBuffer;

SemaphoreHandle_t alarmSemaphore;
static StaticSemaphore_t alarmSemaphoreBuffer;

static void __attribute__((noreturn)) alarm_led_task(void *args) {
    static const gpio_config_t gpioConfig = {
            .pin_bit_mask = BIT(CONFIG_ALARM_LED_PIN),
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
                    level = 1;
                }
                break;
            case ANEMOMETER_DATA_FAIL:
                delay = 100;
                break;
            default:
                delay = 200;
        }
        gpio_set_level((gpio_num_t) CONFIG_ALARM_LED_PIN, level);
        xSemaphoreTake(alarmSemaphore,pdMS_TO_TICKS(delay));
        level = !level;
    }
}


extern "C" void app_main() {
    ESP_UNUSED(app_main);
    ESP_ERROR_CHECK(nvs_flash_init());
    updateSemaphore = xSemaphoreCreateBinaryStatic(&updateSemaphoreBuffer);
    alarmSemaphore = xSemaphoreCreateBinaryStatic(&alarmSemaphoreBuffer);
    xTaskCreate(display_task, "display_task", 2048, nullptr, 10, nullptr);
    xTaskCreate(alarm_led_task, "alarm_led_task", 4048, nullptr, 10, nullptr);
    xTaskCreate(udp_listener_task, "udp_listener_task", 16384, nullptr, 10, nullptr);
}

void elm_init_wifi() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    static wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    static wifi_config_t wifi_config = {
            .sta = {.ssid = CONFIG_ESP_WIFI_SSID,
                    .password = CONFIG_ESP_WIFI_PASSWORD,
                    .scan_method = WIFI_ALL_CHANNEL_SCAN,
                    .bssid_set = false,
                    .channel = 0,
                    .listen_interval = 0
            }
    };
#pragma GCC diagnostic pop

    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

int elm_connect_bind() {
    esp_wifi_start();
    esp_wifi_connect();
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        abort();
    }
    ESP_LOGI(TAG, "Socket created");

    // Set timeout
    struct timeval timeout = {
            .tv_sec = CONFIG_WIFI_STA_TIMEOUT,
            .tv_usec = 0
    };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    static const struct sockaddr_in dest_addr = {
            .sin_len =sizeof(struct sockaddr_in),
            .sin_family = AF_INET,
            .sin_port = htons(CONFIG_NMEA_UDP_PORT),
            .sin_addr= {.s_addr = htonl(INADDR_ANY)},
            .sin_zero={}
    };
    int err = bind(sock, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket bound, port %d", CONFIG_NMEA_UDP_PORT);
    return sock;
}

void elm_unbind_disconnect(int sock) {
    shutdown(sock, SHUT_RDWR);
    closesocket(sock);
    esp_wifi_disconnect();
    esp_wifi_stop();
}

static void __attribute__((noreturn)) udp_listener_task(void *args) {
    ESP_UNUSED(args);
    char rx_buffer[128];
    char addr_str[128];
    elm_init_wifi();
    while (true) {

        int sock = elm_connect_bind();
        struct sockaddr_in source_addr{};
        socklen_t socklen = sizeof(source_addr);

        while (true) {
            ESP_LOGD(TAG, "Waiting for data");
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *) &source_addr, &socklen);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                setWindState(ANEMOMETER_CONN_FAIL);
                break;
            } else {// Data received
                rx_buffer[len] = 0; // Null-terminate received to be a string
                // Get the sender's ip address as string
                inet_ntoa_r(source_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
                ESP_LOGD(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGD(TAG, "%s", rx_buffer);
                if (!parseNmea(rx_buffer)) {
                    ESP_LOGW(TAG, "Malformed NMEA: %s", rx_buffer);
                }

            }
            xSemaphoreGive(updateSemaphore);
        }

        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        elm_unbind_disconnect(sock);
    }
}

void setAngleAlarm(bool value) {
    if (value != windData.angleAlarm) {
        windData.angleAlarm = value;
        xSemaphoreGive(alarmSemaphore);
    }
}

void setWindState(anemometer_state_t value) {
    if (value != windData.state) {
        windData.state = value;
        xSemaphoreGive(alarmSemaphore);
    }
}
