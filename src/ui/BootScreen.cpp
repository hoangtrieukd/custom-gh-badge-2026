#include "BootScreen.h"
#include "Theme.h"
#include "UiKit.h"

void BootScreen::draw(Adafruit_ST7789 &tft) const
{
    tft.fillScreen(COL_BG);

    // Simple badge-style mark made from vector shapes.
    // This is an original placeholder graphic, not the
    // official GitHub logo artwork.
    int cx = SCREEN_WIDTH / 2;
    int cy = 91;

    tft.fillRoundRect(cx - 35, cy - 35, 70, 70, 18, COL_PANEL2);
    tft.drawRoundRect(cx - 35, cy - 35, 70, 70, 18, COL_CYAN);
    tft.fillCircle(cx, cy - 5, 17, COL_WHITE);
    tft.fillCircle(cx - 6, cy - 8, 3, COL_BG);
    tft.fillCircle(cx + 6, cy - 8, 3, COL_BG);
    tft.fillRoundRect(cx - 9, cy + 4, 18, 5, 2, COL_BG);

    ui::drawCenteredText(tft, "GITHUB UNIVERSE", 145, 2, COL_WHITE);
    ui::drawCenteredText(tft, "BADGE", 169, 2, COL_CYAN);

    // Animated progress indicator.
    tft.drawRoundRect(75, 205, 170, 8, 4, COL_MUTED);
    int fillWidth = 12 + (frame_ % 9) * 18;
    if (fillWidth > 164)
        fillWidth = 164;
    tft.fillRoundRect(78, 208, fillWidth, 2, 1, COL_GREEN);
}

void BootScreen::start(Adafruit_ST7789 &tft, unsigned long now)
{
    startedAt_ = now;
    lastFrameAt_ = 0;
    frame_ = 0;
    draw(tft);
}

bool BootScreen::update(Adafruit_ST7789 &tft, unsigned long now)
{
    if (now - lastFrameAt_ >= 120)
    {
        lastFrameAt_ = now;
        frame_++;
        draw(tft);
    }

    // Boot screen lasts about 2 seconds.
    return (now - startedAt_) >= 2000;
}