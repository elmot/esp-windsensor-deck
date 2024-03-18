
#include "libraries/pico_display_2/pico_display_2.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "rgbled.hpp"
#include <hardware/uart.h>
#include <memory>
#include <hardware/sync.h>

using namespace pimoroni;

[[noreturn]] void display_run();

/*
 Pico Display pins:

 LED_R      GP6     9
 LED_G      GP7     10
 LED_B      GP8     11

 SWA        GP12    16
 SWB        GP13    17
 SWX        GP14    19
 SWY        GP15    20

 LCD_TE     GP21    27
 BGLGHT_EN  GP20    26
 LCD_MOSI   GP19    25
 LCD_SCLK   GP18    24
 LCD_CS     GP17    22
 LCD_DC     GP16    21

 UART_RX    GP1     2
  */

#define UART uart0
#define UART_IRQ UART0_IRQ
//#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define BAUD_RATE 625000

std::unique_ptr<ST7789> st7789;
std::unique_ptr<PicoGraphics_PenRGB332> graphics;
Pen BG;
Pen DRAW_COLOR;
static volatile uint8_t rxBuff[256];
static volatile uint8_t rxIndex;
static volatile uint8_t readIndex;

void on_uart_rx() {
    static uint8_t c;
    while (uart_is_readable(UART)) {
        uart_read_blocking(UART, &c,1);
        rxBuff[rxIndex++] = c;
        if(rxIndex == readIndex) {
            readIndex++;
        }
    }
}

int main() {
    static RGBLED led(PicoDisplay2::LED_R, PicoDisplay2::LED_G, PicoDisplay2::LED_B);
    led.set_rgb(150, 0, 255);
    uart_init(UART, BAUD_RATE);
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    uart_set_irq_enables(UART,true, false);
    irq_set_enabled(UART_IRQ, true);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
//    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_fifo_enabled(UART, false);
    sleep_ms(50);
    st7789 = std::make_unique<ST7789>(320, 240, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));
    graphics = std::make_unique<PicoGraphics_PenRGB332>(st7789->width, st7789->height, nullptr);
    BG = graphics->create_pen(155, 255, 00);
    DRAW_COLOR = graphics->create_pen(0, 0, 0);
    Pen BLUE = graphics->create_pen(0, 0, 50);
    graphics->set_pen(BG);
    graphics->clear();
    graphics->set_pen(DRAW_COLOR);
    graphics->set_pen(BLUE);
    graphics->text("Yanus wind display simulator", Point(0, 0), 320);

    // update screen
    st7789->update(graphics.get());
    led.set_rgb(0, 255, 0);
    display_run();
}

enum DISP_STATE {
    D_ESCAPE, D_BRIGHTNESS, D_WRITE, D_ERROR
};
#define ESC_BYTE (0xAA)
#define ESC_ESC_BYTE (0xA9)
#define ESC_START_BYTE (0xA0)

static const size_t DISP_WIDTH = 240;
static const size_t DISP_HEIGHT = 128;
static const int32_t BASE_X = 0;
static const int32_t BASE_Y = 40;


static void drawByte(int32_t &x, int32_t &y, uint8_t data) {
    for (uint8_t mask = 128, shiftX = 0; shiftX < 8; mask >>= 1, ++shiftX) {
        if (data & mask) {
            graphics->set_pixel(Point(x + BASE_X + shiftX, y + BASE_Y));
        }
    }
    x += 8;
    if (x == DISP_WIDTH) {
        x = 0;
        y++;
    }
}
uint8_t readUartByte() {
    uint8_t c;
    while(true) {
        irq_set_enabled(UART0_IRQ,false);
        if(rxIndex!=readIndex) {
            c = rxBuff[readIndex++];
            irq_set_enabled(UART0_IRQ,true);
            return c;
        }
        irq_set_enabled(UART0_IRQ,true);
    }
}


[[noreturn]] void display_run() {

    uint8_t c;
    int32_t x = 0;
    int32_t y = 0;

    enum DISP_STATE state = D_ERROR;
    while (true) {
        c = readUartByte();

        if (c == ESC_BYTE) {
            state = D_ESCAPE;
        } else {

            switch (state) {
                case D_ESCAPE:
                    switch (c) {
                        case ESC_ESC_BYTE:
                            drawByte(x, y, ESC_BYTE);
                            state = D_WRITE;
                            break;
                        case ESC_START_BYTE:
                            state = D_BRIGHTNESS;
                            break;
                        default:
                            state = D_ERROR;
                            break;
                    }
                    break;
                case D_BRIGHTNESS:
                    if (c < 101) {
                        st7789->set_backlight(155 + c);
                        state = D_WRITE;
                        x = y = 0;
                    } else {
                        state = D_ERROR;
                    }
                    break;
                case D_WRITE:
                    drawByte(x, y, c);
                    break;
                case D_ERROR:
                    break;
            }
            if (y == DISP_HEIGHT) {
                x = y = 0;
                st7789->update(graphics.get());
                graphics->set_pen(BG);
                for(int32_t rectY = BASE_Y; rectY < BASE_Y+DISP_HEIGHT;rectY++) {
                    graphics->set_pixel_span(Point(BASE_X,rectY), DISP_WIDTH);
                }
                graphics->set_pen(DRAW_COLOR);
                state = D_ERROR;
            }
        }
    }
}
