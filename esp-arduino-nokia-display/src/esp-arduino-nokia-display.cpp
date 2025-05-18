#include <Arduino.h>
#ifdef ARDUINO_ARCH_ESP32
#include "pinout-esp32.h"
#include <WiFi.h>

#else
#ifdef ARDUINO_ARCH_ESP8266
#include "pinout-esp.h"
#include <ESP8266WiFi.h>
#else
#error "Unknown Board"
#endif
#endif

#include <WiFiUdp.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include "nmea-wind-parser.hpp"


#define NO_WARNINGS "$PEWWT,NONE"
#define NO_WARNINGS_PREFIX_LEN 7
#define NO_WARNINGS_LEN 11
#define BUFFER_SIZE 8192


const char *ssid = "yanus-wind";  // WiFi SSID
const char *pw = "EglaisEglais"; // and WiFi PASSWORD
const int port = 9000;

WiFiUDP udp;

 uint8_t buffer[BUFFER_SIZE + 1];
char txtBuffer[16];
uint16_t i1=0;

bool angleAlarm = false;

void setAngleAlarm(bool flag) {
  angleAlarm = flag;
}

anemometer_state_t state;
void setWindState(anemometer_state_t value) {
  state = value;
}

wind_data_t windData;

Adafruit_PCD8544 display = Adafruit_PCD8544(SPI_SCLK,SPI_DIN,SPI_DC,SPI_CS, SPI_RST);

void paintStatus(const char * text) {
  display.clearDisplay();
  display.setCursor(0,31);
  display.setFont(&FreeSans9pt7b);
  display.println(text);
  display.display();
}

void setup()   {
  Serial.begin(115200);
  Serial.println("Start NMEA Display");
  display.begin();

  display.setContrast(75);

  paintStatus("Init...");

  Serial.println("Starting UDP Server");
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, pw);
  udp.begin(port); // start UDP server 
  delay(500);
  display.setTextSize(1);
  display.setTextColor(BLACK);
}


void loop() {

  if (WiFi.status() != WL_CONNECTED) {
      paintStatus("Connect...");
      delay(500);
  }
  int packetSize = udp.parsePacket();
  if(packetSize>0) {
    int len = udp.read(buffer, BUFFER_SIZE);
    if (len > 0) {
      buffer[len] = 0;
    }

    // now send to UART:
    Serial.write(buffer, packetSize);
    bool success = parseNmea((const char*)buffer);
    if(!success) {
      paintStatus("Fmt error");
      delay(500);
      return;
    }
    if(state == ANEMOMETER_DATA_FAIL) {
      paintStatus("Data error");
      delay(500);
      return;
    }
    char txtBuf[16];
    int16_t x1, y1;
    uint16_t w, h;
    int absAngle;
    int speedX;

    display.clearDisplay();
    display.setFont(&FreeSans9pt7b);
    if(windData.windAngle > 180) {
      absAngle = 360 - windData.windAngle;
      snprintf(txtBuf,15,"%4.1fm/s <", windData.windSpdMps);
      display.getTextBounds(txtBuf, 0, 31, &x1, &y1, &w, &h);
      speedX = 84 - w - x1;
    } else {
      absAngle = windData.windAngle;
      snprintf(txtBuf,15,"> %4.1fm/s", windData.windSpdMps);
      speedX = 0;
    }
    display.setCursor(speedX, 47);
    display.write(txtBuf);


    snprintf(txtBuf,15,"%d", absAngle);
    display.setFont(&FreeMonoBold24pt7b);
    display.getTextBounds(txtBuf, 0, 31, &x1, &y1, &w, &h);
    display.setCursor((84 - w - x1)/2, 31);
    display.write(txtBuf);


    display.invertDisplay(angleAlarm);
    display.display();
  }
  
}