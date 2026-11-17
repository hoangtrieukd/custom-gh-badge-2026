#include "UiKit.h"
#include "Theme.h"

namespace ui
{
    void drawCenteredText(Adafruit_ST7789 &tft, const char *text, int y,
                          uint8_t size, uint16_t colour)
    {
        tft.setTextSize(size);
        tft.setTextColor(colour);
        int16_t x1, y1;
        uint16_t w, h;
        tft.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
        int x = (SCREEN_WIDTH - (int)w) / 2;
        tft.setCursor(x, y);
        tft.print(text);
    }

    void drawHeader(Adafruit_ST7789 &tft, const char *section)
    {
        tft.fillRect(0, 0, SCREEN_WIDTH, 34, COL_PANEL);
        tft.setTextSize(1);
        tft.setTextColor(COL_CYAN);
        tft.setCursor(12, 8);
        tft.print("GITHUB");
        tft.setTextColor(COL_WHITE);
        tft.setCursor(12, 19);
        tft.print("UNIVERSE BADGE");

        tft.setTextColor(COL_MUTED);
        int16_t x1, y1;
        uint16_t w, h;
        tft.getTextBounds(section, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor(SCREEN_WIDTH - (int)w - 12, 13);
        tft.print(section);
    }

    void drawFooter(Adafruit_ST7789 &tft, const char *hint)
    {
        tft.fillRect(0, 218, SCREEN_WIDTH, 22, COL_PANEL);
        tft.setTextSize(1);
        tft.setTextColor(COL_MUTED);
        tft.setCursor(10, 226);
        tft.print(hint);
    }
}