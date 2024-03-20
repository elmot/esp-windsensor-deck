#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <esp_wifi.h>

#include "elm_display.hpp"
#include "sdkconfig.h"
//todo watchdog
//todo initial screen
//todo normal reconnect
//todo brightness buttons
//todo backlight
//todo semaphore
static const char *TAG = "NMEA DISPLAY";

struct wind_state state = {
        .anemState=ANEMOMETER_OK,
        .windAngle=27,
        .windSpdMps=6,
        .backLightPercent =50,
        .angleAlarm=true,
        .timestamp = 0};

static void __attribute__((noreturn)) display_task(void *arg) {
    ESP_UNUSED(TAG);
    ESP_UNUSED(arg);
    lcdInit();
    lcdSplash();
    while (true) {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 1500 * xPortGetTickRateHz() / 1000);
        lcdMainScreenUpdatePicture();
    }
}

static void udp_listener_task(void *arg);


static esp_netif_t *elm_sta_netif = nullptr;

void elm_wifi_start() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    esp_netif_config.if_desc = "Yanus_deck_display";
    esp_netif_config.route_prio = 128;

    elm_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start());
}

esp_err_t wifi_sta_connect() {

    ESP_LOGI(TAG, "Start WiFi connect.");
    elm_wifi_start();
    wifi_config_t wifiConfig = {
            .sta = {
                    .ssid = CONFIG_ESP_WIFI_SSID,
                    .password = CONFIG_ESP_WIFI_PASSWORD,
                    .scan_method = WIFI_FAST_SCAN,
                    .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
                    .threshold= {.rssi = -127,
                                 .authmode = WIFI_AUTH_WPA2_PSK},
            },
    };
    ESP_LOGI(TAG, "Connecting to %s...", wifiConfig.sta.ssid);
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        state.anemState = ANEMOMETER_CONN_FAIL;
        ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
    }
    return ret;
}

extern "C" void app_main() {
    ESP_UNUSED(app_main);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    xTaskCreate(display_task, "display_task", 2048, nullptr, 10, nullptr);
    xTaskCreate(udp_listener_task, "udp_listener_task", 16384, nullptr, 10, nullptr);

}


static void udp_listener_task(void *args) {
    ESP_UNUSED(args);
    char rx_buffer[128];
    char addr_str[128];
    static const struct sockaddr_in dest_addr = {
            .sin_len =sizeof(struct sockaddr_in),
            .sin_family = AF_INET,
            .sin_port = htons(CONFIG_NMEA_UDP_PORT),
            .sin_addr= {.s_addr = htonl(INADDR_ANY)},
            .sin_zero={}
    };

    while (true) {

        wifi_sta_connect();
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        int err = bind(sock, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", CONFIG_NMEA_UDP_PORT);

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);


        while (true) {
            ESP_LOGD(TAG, "Waiting for data");
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *) &source_addr, &socklen);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                state.anemState = ANEMOMETER_CONN_FAIL;
                break;
            }
                // Data received
            else {
                // Get the sender's ip address as string
                if (source_addr.ss_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *) &source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
                } else if (source_addr.ss_family == PF_INET6) {
                    inet6_ntoa_r(((struct sockaddr_in6 *) &source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate received to be a string
                ESP_LOGD(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGD(TAG, "%s", rx_buffer);
                if (!parseNmea(rx_buffer)) {
                    ESP_LOGW(TAG, "Malformed NMEA: %s", rx_buffer);
                }

            }
        }

        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
        esp_netif_destroy(elm_sta_netif);
    }
}
