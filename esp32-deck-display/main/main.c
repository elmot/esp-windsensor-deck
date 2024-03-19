#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

//#include <sys/param.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "freertos/event_groups.h"
//#include "esp_system.h"
//#include "protocol_examples_common.h"

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <esp_wifi.h>


#include "elm_display.h"

//todo watchdog
static const char *TAG = "NMEA DISPLAY";

#define UDP_NMEA_PORT (9000)

struct wind_state state = {
        .backLightPercent =50,
        .anemState=ANEMOMETER_OK,
        .angleAlarm=true,
        .windAngle=27,
        .windSpdMps=6,
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

/*
        { //todo remove
            state.windAngle += 1;
            state.windSpdMps += 1;
            state.angleAlarm = !state.angleAlarm;
            state.anemState = (state.anemState + 1) % 3;
        }
*/

    }
}

static void udp_listener_task(void *arg);


esp_err_t wifi_sta_do_connect(wifi_config_t wifi_config, bool wait) {
    if (wait) {
//        s_semph_get_ip_addrs = xSemaphoreCreateBinary();
//        if (s_semph_get_ip_addrs == NULL) {
//            return ESP_ERR_NO_MEM;
//        }
    }
//    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &example_handler_on_wifi_disconnect, NULL));
//    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &example_handler_on_sta_got_ip, NULL));
//    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &example_handler_on_wifi_connect, s_example_sta_netif));

    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
        return ret;
    }
//    if (wait) {
//        ESP_LOGI(TAG, "Waiting for IP(s)");
//        xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
//        if (s_retry_num > CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY) {
//            return ESP_FAIL;
//        }
//    }
    return ESP_OK;
}

static esp_netif_t *s_example_sta_netif = NULL;

void example_wifi_start(void) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    esp_netif_config.if_desc = "Yanus_deck_display";
    esp_netif_config.route_prio = 128;
    s_example_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t wifi_sta_connect(void) {

    ESP_LOGI(TAG, "Start example_connect.");
    example_wifi_start();
    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = "yanus-wind",  //todo config
                    .password = "EglaisEglais", //todo config
                    .scan_method = WIFI_FAST_SCAN, //todo config
                    .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
                    .threshold.rssi = -127, //todo config
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };
    return wifi_sta_do_connect(wifi_config, true);
}

void app_main() {
    ESP_UNUSED(app_main);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    xTaskCreate(display_task, "display_task", 2048, NULL, 10, NULL);
    xTaskCreate(udp_listener_task, "udp_listener_task", 16384, NULL, 10, NULL);

}


static void udp_listener_task(void *args) {
    ESP_UNUSED(args);
    char rx_buffer[128];
    char addr_str[128];
    static const struct sockaddr_in dest_addr = {
            .sin_addr.s_addr = htonl(INADDR_ANY),
            .sin_family = AF_INET,
            .sin_port = htons(UDP_NMEA_PORT)
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
        ESP_LOGI(TAG, "Socket bound, port %d", UDP_NMEA_PORT);

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);


        while (1) {
            ESP_LOGI(TAG, "Waiting for data");
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *) &source_addr, &socklen);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
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

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);
                if(!scanAlarm(rx_buffer)) {//todo display timeout
                    if(!scanWind(rx_buffer)) {
                        ESP_LOGW(TAG, "Malformed NMEA: %s", rx_buffer);
                    }
                }

            }
        }

        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
    }
}
