#include "Launcher.h"
#include <string.h>
#include "Theme.h"
#include "UiKit.h"

namespace
{
    const int kColumns = 2;
    const int kCardWidth = 145;
    const int kCardHeight = 72;

    // Text area right of the icon tile.
    const int kTextOffsetX = 57;
    const int kTextRightPadding = 8;
    const int kSubtitleOffsetY = 30;
    const int kSubtitleLineHeight = 11;
    const int kSubtitleMaxLines = 2;

    struct Slot
    {
        int x;
        int y;
    };

    const Slot kSlots[kLauncherSlots] = {
        {12, 47}, {163, 47}, {12, 127}, {163, 127}};

    // Draws text (size 1, 6 px per character) wrapped at word boundaries
    // so it never exceeds maxWidth. If the text does not fit in maxLines,
    // the last line ends with "...".
    void drawWrappedText(Adafruit_ST7789 &tft, const char *text, int x, int y,
                         int maxWidth, int maxLines, int lineHeight)
    {
        const int charWidth = 6;
        int maxChars = maxWidth / charWidth;
        if (maxChars < 4)
            return;
        if (maxChars > 31)
            maxChars = 31;

        const char *p = text;

        for (int lineNo = 0; lineNo < maxLines; lineNo++)
        {
            while (*p == ' ')
                p++;

            int remaining = (int)strlen(p);
            if (remaining == 0)
                break;

            int len = remaining;
            if (remaining > maxChars)
            {
                len = maxChars;
                for (int i = maxChars; i > 0; i--)
                {
                    if (p[i] == ' ')
                    {
                        len = i;
                        break;
                    }
                }
            }

            char line[40];
            bool truncated = (lineNo == maxLines - 1) && (len < remaining);

            if (truncated && len > maxChars - 3)
                len = maxChars - 3;

            memcpy(line, p, (size_t)len);
            line[len] = '\0';
            if (truncated)
                strcat(line, "...");

            tft.setCursor(x, y + lineNo * lineHeight);
            tft.print(line);

            p += len;
        }
    }

    void drawCard(Adafruit_ST7789 &tft, const AppInfo &info, bool selected,
                  const Slot &slot)
    {
        const int x = slot.x;
        const int y = slot.y;
        const int w = kCardWidth;
        const int h = kCardHeight;

        uint16_t border = selected ? COL_CYAN : COL_PANEL2;
        uint16_t background = selected ? COL_SELECTED : COL_PANEL;

        tft.fillRoundRect(x, y, w, h, 9, background);
        tft.drawRoundRect(x, y, w, h, 9, border);

        // Icon tile
        tft.fillRoundRect(x + 10, y + 10, 38, 38, 8, info.accent);
        tft.setTextColor(COL_BLACK);
        tft.setTextSize((strlen(info.symbol) > 1) ? 1 : 2);

        int16_t x1, y1;
        uint16_t tw, th;
        tft.getTextBounds(info.symbol, 0, 0, &x1, &y1, &tw, &th);
        tft.setCursor(x + 10 + (38 - (int)tw) / 2,
                      y + 10 + (38 - (int)th) / 2);
        tft.print(info.symbol);

        tft.setTextSize(1);
        tft.setTextColor(selected ? COL_CYAN : COL_WHITE);
        tft.setCursor(x + kTextOffsetX, y + 14);
        tft.print(info.name);

        tft.setTextColor(COL_MUTED);
        drawWrappedText(tft, info.subtitle, x + kTextOffsetX,
                        y + kSubtitleOffsetY,
                        w - kTextOffsetX - kTextRightPadding,
                        kSubtitleMaxLines, kSubtitleLineHeight);

        if (selected)
        {
            tft.fillCircle(x + w - 12, y + h - 12, 3, COL_CYAN);
        }
    }
}

void Launcher::draw(AppContext &context, App *const *apps, size_t count) const
{
    Adafruit_ST7789 &tft = context.tft;

    tft.fillScreen(COL_BG);
    ui::drawHeader(tft, "HOME");

    for (size_t i = 0; i < count && i < kLauncherSlots; i++)
    {
        drawCard(tft, apps[i]->info(), i == selected_, kSlots[i]);
    }

    ui::drawFooter(tft, "D-PAD: MOVE   SELECT: OPEN   BACK: EXIT");
}

bool Launcher::move(AppContext &context, App *const *apps, size_t count,
                    AppButton button)
{
    int dx = 0;
    int dy = 0;

    switch (button)
    {
    case AppButton::Up:
        dy = -1;
        break;
    case AppButton::Down:
        dy = 1;
        break;
    case AppButton::Left:
        dx = -1;
        break;
    case AppButton::Right:
        dx = 1;
        break;
    default:
        return false;
    }

    if (count == 0)
        return true;

    const int rows = ((int)count + kColumns - 1) / kColumns;
    int col = (int)(selected_ % kColumns) + dx;
    int row = (int)(selected_ / kColumns) + dy;

    if (col < 0)
        col = kColumns - 1;
    if (col >= kColumns)
        col = 0;
    if (row < 0)
        row = rows - 1;
    if (row >= rows)
        row = 0;

    size_t index = (size_t)(row * kColumns + col);
    if (index >= count)
        index = count - 1;
    selected_ = index;

    draw(context, apps, count);

    context.console.print("APP SELECTED: ");
    context.console.println(apps[selected_]->name());
    return true;
}