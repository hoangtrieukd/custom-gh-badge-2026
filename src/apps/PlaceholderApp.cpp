#include "PlaceholderApp.h"
#include <stdio.h>
#include "../ui/UiKit.h"

PlaceholderApp::PlaceholderApp(const AppInfo &info) : info_(info) {}

void PlaceholderApp::onEnter(AppContext &context)
{
    enterCount_++;
    buttonCount_ = 0;
    hasLastButton_ = false;

    context.console.print("ENTER ");
    context.console.print(info_.name);
    context.console.print(" (opens: ");
    context.console.print(enterCount_);
    context.console.println(")");
}

void PlaceholderApp::draw(AppContext &context)
{
    Adafruit_ST7789 &tft = context.tft;

    tft.fillScreen(COL_BG);
    ui::drawHeader(tft, info_.name);

    tft.fillRoundRect(24, 57, 272, 112, 12, COL_PANEL);
    tft.drawRoundRect(24, 57, 272, 112, 12, info_.accent);

    ui::drawCenteredText(tft, info_.name, 82, 2, COL_WHITE);
    ui::drawCenteredText(tft, "COMING SOON", 116, 2, info_.accent);
    ui::drawCenteredText(tft, "This app will be ported", 145, 1, COL_MUTED);

    drawStatus(tft);

    ui::drawFooter(tft, "BACK: RETURN TO LAUNCHER");
}

void PlaceholderApp::drawStatus(Adafruit_ST7789 &tft) const
{
    char line[48];

    tft.fillRect(0, 176, SCREEN_WIDTH, 38, COL_BG);

    snprintf(line, sizeof(line), "OPENED: %u   PRESSES: %u",
             (unsigned)enterCount_, (unsigned)buttonCount_);
    ui::drawCenteredText(tft, line, 182, 1, COL_WHITE);

    snprintf(line, sizeof(line), "LAST INPUT: %s",
             hasLastButton_ ? appButtonName(lastButton_) : "-");
    ui::drawCenteredText(tft, line, 197, 1, info_.accent);
}

bool PlaceholderApp::onButton(AppContext &context, AppButton button)
{
    // Let the AppManager handle BACK (exit).
    if (button == AppButton::Back)
        return false;

    buttonCount_++;
    lastButton_ = button;
    hasLastButton_ = true;

    context.console.print("INPUT -> ");
    context.console.print(info_.name);
    context.console.print(": ");
    context.console.println(appButtonName(button));

    drawStatus(context.tft);
    return true;
}

void PlaceholderApp::onExit(AppContext &context)
{
    context.console.print("EXIT ");
    context.console.println(info_.name);
}