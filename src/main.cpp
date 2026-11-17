#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "app/App.h"
#include "app/AppManager.h"
#include "apps/GamesApp.h"
#include "apps/ToolsApp.h"
#include "apps/GithubApp.h"
#include "apps/SettingsApp.h"
#include "ui/BootScreen.h"

// =====================================================
// GITHUB BADGE ESP32 - MILESTONE 2
// Application framework: AppManager + Launcher + apps
// =====================================================

// TFT wiring (existing working configuration)
#define TFT_CS 8
#define TFT_RST 10
#define TFT_DC 9
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_MISO 13

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

// Keep this serial alias because it is part of the user's
// currently working ESP32-S3 project configuration.
#define Serial HWCDCSerial

// =====================================================
// BUTTONS
// =====================================================

#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7
#define BUTTON_SELECT 15
#define BUTTON_MENU 0  // Built-in BOOT button
#define BUTTON_BACK 16 // User-confirmed BACK wiring

#define DEBOUNCE_MS 40
#define SELECT_LONG_PRESS_MS 800

struct Button
{
    AppButton id;
    uint8_t pin;
    bool lastReading;
    bool stableState;
    unsigned long lastChangeTime;
    unsigned long pressStartTime;
    bool longPressHandled;
};

Button buttons[] = {
    {AppButton::Up, BUTTON_UP, HIGH, HIGH, 0, 0, false},
    {AppButton::Down, BUTTON_DOWN, HIGH, HIGH, 0, 0, false},
    {AppButton::Left, BUTTON_LEFT, HIGH, HIGH, 0, 0, false},
    {AppButton::Right, BUTTON_RIGHT, HIGH, HIGH, 0, 0, false},
    {AppButton::Select, BUTTON_SELECT, HIGH, HIGH, 0, 0, false},
    {AppButton::Menu, BUTTON_MENU, HIGH, HIGH, 0, 0, false},
    {AppButton::Back, BUTTON_BACK, HIGH, HIGH, 0, 0, false}};

const size_t BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

// =====================================================
// SYSTEM OBJECTS
// =====================================================

AppContext context(tft, Serial);
AppManager manager(context);
BootScreen bootScreen;

// One instance per launcher entry, registered in setup().
GamesApp gamesApp;
ToolsApp toolsApp;
GithubApp githubApp;
SettingsApp settingsApp;

bool booting = true;

// =====================================================
// BOOT
// =====================================================

void startBoot()
{
    booting = true;
    bootScreen.start(tft, millis());
    Serial.println("UI: boot screen");
}

void endBoot(const char *reason)
{
    booting = false;
    manager.showLauncher();
    Serial.println(reason);
}

// =====================================================
// INPUT ACTIONS
// =====================================================

void dispatchButton(AppButton button)
{
    Serial.print("{\"event\":\"pressed\",\"button\":\"");
    Serial.print(appButtonName(button));
    Serial.println("\"}");

    // Any press (including MENU/HOME) skips the boot intro.
    if (booting)
    {
        endBoot("UI: boot skipped");
        return;
    }

    manager.handleButton(button);
}

// =====================================================
// BUTTON SCANNING
// =====================================================

void scanButtons(unsigned long now)
{
    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        Button &button = buttons[i];
        bool reading = digitalRead(button.pin);

        if (reading != button.lastReading)
        {
            button.lastChangeTime = now;
            button.lastReading = reading;
        }

        if ((now - button.lastChangeTime) >= DEBOUNCE_MS)
        {
            if (reading != button.stableState)
            {
                button.stableState = reading;

                if (reading == LOW)
                {
                    button.pressStartTime = now;
                    button.longPressHandled = false;

                    if (button.id != AppButton::Select)
                    {
                        dispatchButton(button.id);
                    }
                }
                else if (button.id == AppButton::Select)
                {
                    if (!button.longPressHandled)
                    {
                        dispatchButton(AppButton::Select);
                    }
                }
            }
        }

        // Long SELECT = HOME. It is detected while held.
        if (button.id == AppButton::Select &&
            button.stableState == LOW &&
            !button.longPressHandled &&
            now - button.pressStartTime >= SELECT_LONG_PRESS_MS)
        {
            button.longPressHandled = true;
            dispatchButton(AppButton::Home);
        }
    }
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);
    delay(300);

    SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

    tft.init(240, 320);
    tft.setRotation(1);
    tft.invertDisplay(false);

    for (size_t i = 0; i < BUTTON_COUNT; i++)
    {
        pinMode(buttons[i].pin, INPUT_PULLUP);
        buttons[i].lastReading = digitalRead(buttons[i].pin);
        buttons[i].stableState = buttons[i].lastReading;
        buttons[i].lastChangeTime = millis();
    }

    // Registration order = launcher order (2 x 2 grid).
    bool ok = true;
    ok &= manager.registerApp(&gamesApp);
    ok &= manager.registerApp(&toolsApp);
    ok &= manager.registerApp(&githubApp);
    ok &= manager.registerApp(&settingsApp);

    Serial.println();
    Serial.println("==============================");
    Serial.println(" GITHUB BADGE ESP32");
    Serial.println(" MILESTONE 2: APP FRAMEWORK");
    Serial.println("==============================");
    Serial.println("BACK = GPIO 16");
    Serial.println("MENU = BOOT / GPIO 0");
    Serial.println("HOME = hold SELECT for 800 ms");
    Serial.print("Apps registered: ");
    Serial.print(manager.appCount());
    Serial.println(ok ? "" : "  (REGISTRATION FAILED)");

    startBoot();
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
    unsigned long now = millis();
    context.now = now;

    scanButtons(now);

    if (booting)
    {
        if (bootScreen.update(tft, now))
        {
            endBoot("UI: launcher ready");
        }
    }
    else
    {
        manager.update();
    }
}