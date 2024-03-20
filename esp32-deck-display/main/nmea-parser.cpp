#include "nmea-wind-parser.hpp"
#include <cstring>
#include <cmath>

static inline int charToHex(char c) {
    return (c < '0') ? -1 :
           (c <= '9') ? (c - '0') :
           (c < 'A') ? -1 :
           (c <= 'F') ? (c - 'A' + 10) :
           (c < 'a') ? -1 :
           (c <= 'f') ? (c - 'a' + 10) : -1;
}

static bool isNmeaFrameOk(const char *nmeaString) {
    int idx;
    if (nmeaString == nullptr || nmeaString[0] != '$') return false;
    unsigned char checkSum = 0;
    for (idx = 1; (idx < 100); ++idx) {
        switch (nmeaString[idx]) {
            case 0:
                return false;
            case '*':
                ++idx;
                goto checkHex;
            default:
                checkSum ^= nmeaString[idx];
        }
    }
    checkHex:
    if (charToHex(nmeaString[idx++]) != checkSum >> 4u) return false;
    return charToHex(nmeaString[idx]) == (checkSum & 0xF);
}

enum parse_result {
    OK, MISSING, WRONG
};

static enum parse_result parseFloat(const char *&ptr, float &parsed) {
    if (*ptr == ',') return MISSING;
    float result = 0;
    for (; true; ptr++) {
        if (*ptr >= '0' && *ptr <= '9') {
            result = result * 10 + (float) *ptr - '0';
        } else
            switch (*ptr) {
                case ',':
                case 0:
                    parsed = result;
                    return OK;
                case '.':
                    goto fraction;
                default:
                    return WRONG;
            }
    }
    fraction:
    ptr++;
    for (float divider = 0.1; true; ptr++) {
        if (*ptr >= '0' && *ptr <= '9') {
            result += divider * (float) (*ptr - '0');
            divider /= 10;
        } else
            switch (*ptr) {
                case ',':
                case 0:
                    parsed = result;
                    return OK;
                default:
                    return WRONG;
            }
    }
}

bool parseNmea(const char *nmeaString) {
    if (!isNmeaFrameOk(nmeaString)) return false;
    const char *NO_WARNING = "$PEWWT,NONE*";
    if (strncmp(NO_WARNING, nmeaString, 7) == 0) {
        state.timestamp = xTaskGetTickCount();
        state.angleAlarm = strncmp(NO_WARNING, nmeaString, strlen(NO_WARNING)) != 0;
        return true;
    }
    if (nmeaString[1] == 0 || nmeaString[2] == 0) return false;//Instrument type, ignored
    if (strncmp("MWV,", nmeaString + 3, 4) != 0) return false;
    float speed, angle;
    const char *ptr = nmeaString + 7;
    enum anem_state anemState = ANEMOMETER_OK;
    switch (parseFloat(ptr, angle)) {
        case WRONG:
            return false;
        case MISSING:
            anemState = ANEMOMETER_DATA_FAIL;
            break;
        default:
            break;
    }

    if (',' != *(ptr++)) return false;
    if ('R' != *(ptr++)) return false; //only apparent wind angle supported
    if (',' != *(ptr++)) return false;
    switch (parseFloat(ptr, speed)) {
        case WRONG:
            return false;
        case MISSING:
            anemState = ANEMOMETER_DATA_FAIL;
            break;
        default:
            break;
    }
    if (',' != *(ptr++)) return false;
    switch (*(ptr++)) {
        case 'M':
            break;
        case 'K':  //  km/h
            speed /= 3.6;
            break;
        case 'N':  //  knots
            speed /= 1.9438;
            break;
        default:
            return false;
    }
    if (',' != *(ptr++)) return false;
    switch (*ptr) {
        case 'V':
            anemState = ANEMOMETER_DATA_FAIL;
            break;
        case 'A':
            break;
        default:
            return false;
    }
    state.anemState = anemState;
    state.timestamp = xTaskGetTickCount();
    if (anemState != ANEMOMETER_DATA_FAIL) {
        state.windSpdMps = speed;
        state.windAngle = lroundf(angle);
    }
    return true;
}

