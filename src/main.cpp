
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS 8
#define TFT_RST 10
#define TFT_DC 9
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_MISO 13

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("Testing ST7789...");

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.init(240, 320);
  tft.setRotation(1);

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 30);
  tft.println("HELLO!");

  tft.setTextSize(1);
  tft.setCursor(20, 70);
  tft.println("ESP32-S3");

  tft.drawRect(10, 110, 220, 40, ST77XX_GREEN);

  tft.fillRect(20, 170, 50, 50, ST77XX_RED);
  tft.fillRect(80, 170, 50, 50, ST77XX_BLUE);

  Serial.println("ST7789 test complete.");
}

void loop()
{
}
