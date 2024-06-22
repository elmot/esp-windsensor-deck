ESP-32 Wind indicator
====
Counterpart of [ESP32 Wind sensor](https://github.com/elmot/esp-windsensor-mast).
The firmware is assembled for classic ESP32, but also works on ESP32-S3.

![image](https://github.com/elmot/esp-windsensor-deck/assets/5366945/d2b7af12-069d-4ed9-86d5-83281724a1b7)

Communication
---
Connects as a WiFi station and receives NMEA-183 data via UDP port 9000
WiFi parametes are set via *idf.py menuconfig*

Input pins
---
Both could be adjusted using *idf.py menuconfig*

* Backlight up button GPIO18
* Backlight down button GPIO19

Input pins
---
Both could be adjusted using *idf.py menuconfig*

* GPIO5 - Alarm LED
* GPIO4 - Alarm LED inverted

Display
---
Display unit is connected via unidirectional UART(GPIO17).

There are two variants of the display unit:
* STM8 MCU + Winstar WG240128B display. There are multiple clones of it. STM8 firmware is here: [stm8-display-bridge](stm8-display-bridge). Unfortunately STM8 is totally deprecated now. If there is some interest, I'll port it to some cheap ARM-based 5V MCU.
* Emulator on Raspberry Pi Pico + [Pimoroni Pico Display Pack 2.0](https://shop.pimoroni.com/products/pico-display-pack-2-0). See [rpi-display](rpi-display)

How to flash the main unit
----
There are multiple ways to flash ESP32. The easiest way is [esptool-js](https://espressif.github.io/esptool-js/). 

Take binaries from [the latest release](//github.com/elmot/esp-windsensor-deck/releases/latest). 

Flash addresses are
| File                    | Address |
| ----------------------- | ------- |
| bootloader.bin          | 0x1000  |
| partition-table.bin     | 0x8000  |
| esp-windsensor-mast.bin | 0x10000 |
 

 
