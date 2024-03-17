
#include "libraries/pico_display_2/pico_display_2.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "rgbled.hpp"

using namespace pimoroni;

ST7789 st7789(320, 240, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));
PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);
RGBLED led(PicoDisplay2::LED_R, PicoDisplay2::LED_G, PicoDisplay2::LED_B);

int main() {
    Point text_location(5, 7);
    led.set_rgb(0, 0, 255);
    Pen BG = graphics.create_pen(120, 40, 60);
    Pen WHITE = graphics.create_pen(255, 255, 255);
    Pen BLUE = graphics.create_pen(55, 55, 255);
    graphics.set_pen(BG);
    graphics.clear();
    graphics.set_pen(WHITE);
    graphics.circle(Point(100, 100), 88);
    graphics.set_pen(BLUE);
    graphics.text("Hello World", text_location, 320);

    // update screen
    st7789.update(&graphics);
    led.set_rgb(0, 255, 0);
    while (1);

}