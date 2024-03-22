#include <cstring>
#include <cmath>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "elm_display.hpp"

uint8_t brightness = 60; //todo 0

#define BUF_SIZE (1024)
#define DISPLAY_UART (UART_NUM_1)
#define DISPLAY_TX_PIN (17)

#define MAX_LINE_POINTS 200

static const uint8_t SPLASH_PICTURE[] = {
#include "bitmaps/splash.txt"
};

const uint8_t FONT[] = {
#include "bitmaps/font.txt"
};
#define CHAR_BACK_LIGHT 10
#define CHAR_CONN_FAIL 12
#define CHAR_DATA_FAIL 15
#define CHAR_WIFI 16
#define CHAR_BATTERY 11
#define CHAR_ANCHOR_LIGHTS 13
#define CHAR_NAVI_LIGHTS 14

const uint8_t FONT40x48[] = {
#include "bitmaps/font40x48.txt"
};
static const uint8_t BACKGROUND[] = {
#include "bitmaps/background.txt"
};


static uint8_t screen_buffer[SCREEN_WIDTH_BYTES * SCREEN_HEIGHT];

static void displayCopyPict(const uint8_t *background) {
    memcpy(screen_buffer, background, SCREEN_WIDTH_BYTES * SCREEN_HEIGHT);
}

class AffineTransformDisplay {
public:
    static void screenPixel(float x, float y, bool color);

    [[maybe_unused]]void pixel(float x, float y, bool color) const;

    void line(float x1, float y1, float x2, float y2, float width, const char *pattern);

    static void screenLine(float x1, float y1, float x2, float y2, float width, const char *pattern);

    void reset();

    void rotate(float theta);

    void translate(float tx, float ty);

    void rotate(float theta, float anchorX, float anchorY);

    void scale(float sx, float sy);

private:
    float m00 = 0;

    /**
     * The Y coordinate shearing element of the 3x3
     * affine transformation matrix.
     *
     * @serial
     */
    float m10 = 0;

    /**
     * The X coordinate shearing element of the 3x3
     * affine transformation matrix.
     *
     * @serial
     */
    float m01 = 0;

    /**
     * The Y coordinate scaling element of the 3x3
     * affine transformation matrix.
     *
     * @serial
     */
    float m11 = 0;

    /**
     * The X coordinate of the translation element of the
     * 3x3 affine transformation matrix.
     *
     * @serial
     */
    float m02 = 0;

    /**
     * The Y coordinate of the translation element of the
     * 3x3 affine transformation matrix.
     *
     * @serial
     */
    float m12 = 0;

};

static AffineTransformDisplay affineTransformDisplay;

static void displayPaint() {
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
    ESP_ERROR_CHECK(uart_driver_install(DISPLAY_UART, BUF_SIZE * 2, 0, 0, nullptr, 0));
    ESP_ERROR_CHECK(uart_param_config(DISPLAY_UART, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(DISPLAY_UART, DISPLAY_TX_PIN, -1, -1, -1));

}

void lcdSplash() {

    for (int i = 0; i < (2 * SCREEN_WIDTH_BYTES) + 1; i += 2) {
        TickType_t xLastWakeTime = xTaskGetTickCount();
        displayCopyPict(SPLASH_PICTURE);
        for (int j = 0; j < SCREEN_HEIGHT; j++) {
            int k = -j / 5 + i;
            if (k < 0) k = 0;
            for (; k < SCREEN_WIDTH_BYTES; k++) {
                screen_buffer[j * SCREEN_WIDTH_BYTES + k] = 0;
            }
        }
        displayPaint();
        xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SCREEN_UPDATE_TIMEOUT_MS));
    }
}


void enableAlarmLed() {
    //todo
}

void disableAlarmLed() {
    //todo
}

static void
copySprite(unsigned int xBytes, unsigned int y, int wBytes, int h, const uint8_t xorMask, const uint8_t *pict) {
    for (int iy = 0; iy < h; iy++) {
        for (int ix = 0; ix < wBytes; ix++) {
            screen_buffer[(y + iy) * SCREEN_WIDTH_BYTES + xBytes + ix] |= pict[iy * wBytes + ix] ^ xorMask;
        }
    }
}

static inline void charOutput(int charCode, unsigned int xBytes, unsigned int yPos, uint8_t xorMask = 0) {
    copySprite(xBytes, yPos, 4, 32, xorMask, &FONT[32 * 4 * (16 - charCode)]);
}

static inline void bigCharOutput(int charCode, unsigned int xBytes, unsigned int yPos, bool invert = false) {
    copySprite(xBytes, yPos, 5, 48, invert ? 0xff : 0, &FONT40x48[48 * 5 * (19 - charCode)]);
}

static void draw3digits(bool big, unsigned int val, unsigned int xBytes, unsigned int y, int activePos = -1) {
    unsigned char char1 = val / 100;
    unsigned char char2 = val / 10 % 10;
    unsigned char char3 = val % 10;
    if (big) {
        bigCharOutput(char1, xBytes, y, activePos == 0);
        bigCharOutput(char2, xBytes + 5, y, activePos == 1);
        bigCharOutput(char3, xBytes + 10, y, activePos == 2);
    } else {
        charOutput(char1, xBytes, y, activePos == 0 ? 0xFF : 0);
        charOutput(char2, xBytes + 4, y, activePos == 1 ? 0xFF : 0);
        charOutput(char3, xBytes + 8, y, activePos == 2 ? 0xFF : 0);
    }
}

static void lcdDrawScreenData() {
    if (windData.windAngle >= 0) {
        int angle;
        int direction;
        if (windData.windAngle > 180) {
            angle = 360 - windData.windAngle;
            direction = 11;
        } else {
            angle = windData.windAngle;
            direction = 12;
        }
        if (windData.angleAlarm) {
            bigCharOutput(13, 25, 32, false);
            enableAlarmLed();
        } else {
            disableAlarmLed();
        }
        bigCharOutput(direction, 20, 32, false);
        draw3digits(true, angle, 15, 80);
    }

    int speed = lroundf(windData.windSpdMps);
    charOutput(((int) speed / 10) % 10, 17, 0);
    charOutput((int) speed % 10, 21, 0);
}


void lcdMainScreenUpdatePicture() {
    displayCopyPict(BACKGROUND);
    volatile anemometer_state_t state = windData.state;
    if((xTaskGetTickCount() - windData.timestamp ) > INCOMING_DATA_TIMEOUT) {
        state = ANEMOMETER_CONN_TIMEOUT;
    }
    switch (state) {
        case ANEMOMETER_OK:
            affineTransformDisplay.reset();
            affineTransformDisplay.rotate(-(float) windData.windAngle / 180.0f * (float) M_PI, 63, 63);
            affineTransformDisplay.line(63, 127, 63, 63, 3, "1");
            lcdDrawScreenData();
            break;
        case ANEMOMETER_EXPECT_WIFI:
            charOutput(CHAR_WIFI, 6, 38);
            enableAlarmLed();
            break;
        case ANEMOMETER_DATA_FAIL:
            charOutput(CHAR_DATA_FAIL, 6, 38);
            enableAlarmLed();
            break;
        case ANEMOMETER_CONN_TIMEOUT:
        case ANEMOMETER_CONN_FAIL:
        default:
            charOutput(CHAR_CONN_FAIL, 6, 38);
            enableAlarmLed();
            break;
    }
    if (windData.backLightPercent != 0) {
        charOutput(CHAR_BACK_LIGHT, 0, 0);
    }
    displayPaint();
}

void AffineTransformDisplay::screenPixel(float fX, float fY, bool color) {
    int x = (int) roundf(0.5F + fX);
    int y = (int) roundf(0.5F + fY);
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    uint8_t mask = 0x80U >> (x & 7U);
    uint8_t *addr = &screen_buffer[(x >> 3U) + (SCREEN_WIDTH_BYTES * y)];
    if (color) *addr |= mask; else *addr &= (uint8_t) ~mask;

}

 [[maybe_unused]] void AffineTransformDisplay::pixel(float x, float y, bool color) const {
    float tX = 0.5F + m00 * x + m01 * y + m02;
    float tY = 0.5F + m10 * x + m11 * y + m12;
    screenPixel(tX, tY, color);
}

void AffineTransformDisplay::reset() {
    m00 = m11 = 1;
    m10 = m01 = m02 = m12 = 0;
}

void AffineTransformDisplay::rotate(float theta) {
    float aSin = sinf(theta);
    float aCos = cosf(theta);
    float M0;
    float M1;
    M0 = m00;
    M1 = m01;
    m00 = aCos * M0 + aSin * M1;
    m01 = -aSin * M0 + aCos * M1;
    M0 = m10;
    M1 = m11;
    m10 = aCos * M0 + aSin * M1;
    m11 = -aSin * M0 + aCos * M1;
}

void AffineTransformDisplay::translate(float tx, float ty) {
    m02 = tx * m00 + ty * m01 + m02;
    m12 = tx * m10 + ty * m11 + m12;
}

void AffineTransformDisplay::rotate(float theta, float anchorX, float anchorY) {
    translate(anchorX, anchorY);
    rotate(theta);
    translate(-anchorX, -anchorY);
}

void AffineTransformDisplay::line(float x1, float y1, float x2, float y2, float width, const char *pattern) {
    float tX1 = (0.5f + m00 * x1 + m01 * y1 + m02);
    float tY1 = (0.5f + m10 * x1 + m11 * y1 + m12);
    float tX2 = (0.5f + m00 * x2 + m01 * y2 + m02);
    float tY2 = (0.5f + m10 * x2 + m11 * y2 + m12);
    screenLine(tX1, tY1, tX2, tY2, width, pattern);
}

void AffineTransformDisplay::scale(float sx, float sy) {
    m00 *= sx;
    m11 *= sy;
}

void AffineTransformDisplay::screenLine(float fX1, float fY1, float fX2, float fY2, float fWidth, const char *pattern) {
    int x1 = lroundf(fX1);
    int x2 = lroundf(fX2);

    int y1 = lroundf(fY1);
    int y2 = lroundf(fY2);
    int width = lroundf(fWidth);

    bool steep = labs(y2 - y1) > labs(x2 - x1);
    if (steep) {
        std::swap(x1, y1);
    }
    int deltaX = labs(x2 - x1);
    int deltaY = labs(y2 - y1);
    int err = deltaX / 2;
    int y = y1;

    int inc = (x1 < x2) ? 1 : -1;
    int stepY = (y1 < y2) ? 1 : -1;
    const char *patternChar = pattern;

    int pointCount = MAX_LINE_POINTS;
    for (int x = x1; ((x2 - x) * inc >= 0) && (pointCount); x += inc, pointCount--) {

        if (*patternChar == 0) {
            patternChar = pattern;
        }
        unsigned int color = *(patternChar++) & 1u;
        for (int d = 0; d < width; d++) {
            int dy = d - width / 2;
            if (steep) {
                screenPixel((float) (y + dy), (float) x, color);
            } else {
                screenPixel((float) x, (float) (y + dy), color);
            }
        }
        err -= deltaY;
        if (err < 0) {
            y += stepY;
            err += deltaX;
        }
    }
}
