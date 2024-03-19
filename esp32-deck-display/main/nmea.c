#include "freertos/FreeRTOS.h"
#include "elm_display.h"
#include <esp_log.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define TAG_NMEA "NMEA parser"

static bool verifyChecksum(const char *data, unsigned char checksum) {
    for (int i = 0; data[i] != 0 && data[i] != '*'; ++i) {
        checksum ^= data[i];
    }
    return checksum == '$';
}

bool scanAlarm(const char *data) {
    static const char *NO_WARNING = "$PEWWT,NONE*67";
    if (strncmp(NO_WARNING, data, 6) != 0) return false;
    state.timestamp = xTaskGetTickCount();
    if (strcmp(NO_WARNING, data) == 0) {
        state.angleAlarm = false;
    } else {
        state.angleAlarm = true;
    }
    return true;
}

bool scanWind(const char *data) {
    float angle, speed;
    char units, quality;
    int checksum;
    int result = sscanf(data, "$%*2cMWV,%f,R,%f,%c,%c*%2x", &angle, &speed, &units, &quality, &checksum);
    if (result != 5) return false;//todo format error
    if (!verifyChecksum(data, checksum)) return false;
    switch (units) {
        case 'M':
            break;
        case 'K':
            speed /= 3.6f;
            break;
        case 'N':
            speed /= 1.94f;
            break;
        default:
            return false;

    }
    state.timestamp = xTaskGetTickCount();
    state.anemState = quality == 'A' ? ANEMOMETER_OK : ANEMOMETER_DATA_FAIL;
    state.windAngle = (int)angle;
    state.windSpdMps = speed;
    return true;
}
