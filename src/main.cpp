
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "github_logo.h"

// TFT pin configuration
#define TFT_CS 8
#define TFT_RST 10
#define TFT_DC 9
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_MISO 13

// Display dimensions (landscape)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// GitHub username
const char *githubUsername = "hoangtrieukd";

// Initialize ST7789
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("Initializing GitHub Badge...");

    // Initialize SPI
    SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

    // Initialize display
    tft.init(240, 320);
    tft.setRotation(1);

    // Black background
    tft.fillScreen(ST77XX_BLACK);

    // Draw GitHub logo centered horizontally
    int logoX = (SCREEN_WIDTH - GITHUB_LOGO_WIDTH) / 2;
    int logoY = 35;

    tft.drawBitmap(
        logoX,
        logoY,
        github_logo_bitmap,
        GITHUB_LOGO_WIDTH,
        GITHUB_LOGO_HEIGHT,
        ST77XX_WHITE);

    // Draw username
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setTextWrap(false);

    int16_t textX, textY;
    uint16_t textW, textH;

    tft.getTextBounds(
        githubUsername,
        0, 0,
        &textX, &textY,
        &textW, &textH);

    int usernameX = (SCREEN_WIDTH - textW) / 2;
    int usernameY = 180;

    tft.setCursor(usernameX, usernameY);
    tft.print(githubUsername);

    Serial.println("GitHub Badge ready!");
}

void loop()
{
    // Static badge screen; nothing to update.
}
