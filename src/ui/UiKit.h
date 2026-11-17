#pragma once

#include <stdint.h>
#include <Adafruit_ST7789.h>

namespace ui
{
    void drawCenteredText(Adafruit_ST7789 &tft, const char *text, int y,
                          uint8_t size, uint16_t colour);
    void drawHeader(Adafruit_ST7789 &tft, const char *section);
    void drawFooter(Adafruit_ST7789 &tft, const char *hint);
}