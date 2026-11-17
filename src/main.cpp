#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <string.h>

// =====================================================
// GITHUB BADGE ESP32 - MILESTONE 1
// Boot screen + launcher shell
// =====================================================

// TFT wiring (existing working configuration)
#define TFT_CS 8
#define TFT_RST 10
#define TFT_DC 9
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_MISO 13

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

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

void drawLauncher();

struct Button
{
    const char *name;
    uint8_t pin;
    bool lastReading;
    bool stableState;
    unsigned long lastChangeTime;
    unsigned long pressStartTime;
    bool longPressHandled;
};

Button buttons[] = {
    {"UP", BUTTON_UP, HIGH, HIGH, 0, 0, false},
    {"DOWN", BUTTON_DOWN, HIGH, HIGH, 0, 0, false},
    {"LEFT", BUTTON_LEFT, HIGH, HIGH, 0, 0, false},
    {"RIGHT", BUTTON_RIGHT, HIGH, HIGH, 0, 0, false},
    {"SELECT", BUTTON_SELECT, HIGH, HIGH, 0, 0, false},
    {"MENU", BUTTON_MENU, HIGH, HIGH, 0, 0, false},
    {"BACK", BUTTON_BACK, HIGH, HIGH, 0, 0, false}};

const size_t BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

// =====================================================
// COLOUR PALETTE
// =====================================================

const uint16_t COL_BG = 0x1082; // deep blue-grey
const uint16_t COL_PANEL = 0x2126;
const uint16_t COL_PANEL2 = 0x2948;
const uint16_t COL_WHITE = 0xFFFF;
const uint16_t COL_MUTED = 0x9CF3;
const uint16_t COL_CYAN = 0x07FF;
const uint16_t COL_GREEN = 0x07E0;
const uint16_t COL_YELLOW = 0xFFE0;
const uint16_t COL_ORANGE = 0xFD20;
const uint16_t COL_PINK = 0xF81F;
const uint16_t COL_BLACK = 0x0000;

// =====================================================
// APPLICATION MODEL
// These are launcher placeholders for Milestone 1.
// We will replace/expand them with the real 2026 app
// collection as we port applications to the ESP32.
// =====================================================

struct AppEntry
{
    const char *name;
    const char *subtitle;
    const char *symbol;
    uint16_t accent;
};

AppEntry apps[] = {
    {"GAMES", "Play badge games", "G", COL_PINK},
    {"TOOLS", "Badge utilities", "T", COL_CYAN},
    {"GITHUB", "GitHub experience", "GH", COL_GREEN},
    {"SETTINGS", "Device settings", "S", COL_ORANGE}};

const int APP_COUNT = sizeof(apps) / sizeof(apps[0]);

// =====================================================
// UI STATE
// =====================================================

enum ScreenState
{
    SCREEN_BOOT,
    SCREEN_LAUNCHER,
    SCREEN_APP
};

ScreenState currentScreen = SCREEN_BOOT;
int selectedApp = 0;
int openedApp = 0;

unsigned long bootStartedAt = 0;
unsigned long lastBootFrameAt = 0;
uint8_t bootFrame = 0;

// =====================================================
// DRAW HELPERS
// =====================================================

void drawCenteredText(const char *text, int y, uint8_t size,
                      uint16_t colour)
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

void drawHeader(const char *section)
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

void drawFooter(const char *hint)
{
    tft.fillRect(0, 218, SCREEN_WIDTH, 22, COL_PANEL);
    tft.setTextSize(1);
    tft.setTextColor(COL_MUTED);
    tft.setCursor(10, 226);
    tft.print(hint);
}

// =====================================================
// BOOT SCREEN
// =====================================================

void drawBootScreen()
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

    drawCenteredText("GITHUB UNIVERSE", 145, 2, COL_WHITE);
    drawCenteredText("BADGE", 169, 2, COL_CYAN);

    // Animated progress indicator.
    tft.drawRoundRect(75, 205, 170, 8, 4, COL_MUTED);
    int fillWidth = 12 + (bootFrame % 9) * 18;
    if (fillWidth > 164)
        fillWidth = 164;
    tft.fillRoundRect(78, 208, fillWidth, 2, 1, COL_GREEN);
}

void startBoot()
{
    currentScreen = SCREEN_BOOT;
    bootStartedAt = millis();
    lastBootFrameAt = 0;
    bootFrame = 0;
    drawBootScreen();
    Serial.println("UI: boot screen");
}

void updateBoot()
{
    unsigned long now = millis();

    if (now - lastBootFrameAt >= 120)
    {
        lastBootFrameAt = now;
        bootFrame++;
        drawBootScreen();
    }

    // Boot screen lasts about 2 seconds, unless a button
    // is pressed. Any press can skip the intro.
    if (now - bootStartedAt >= 2000)
    {
        currentScreen = SCREEN_LAUNCHER;
        drawLauncher();
        Serial.println("UI: launcher ready");
    }
}

// =====================================================
// LAUNCHER
// =====================================================

void drawAppCard(int index, int x, int y, int w, int h)
{
    bool selected = (index == selectedApp);
    uint16_t border = selected ? COL_CYAN : COL_PANEL2;
    uint16_t background = selected ? 0x294D : COL_PANEL;

    tft.fillRoundRect(x, y, w, h, 9, background);
    tft.drawRoundRect(x, y, w, h, 9, border);

    // Icon tile
    tft.fillRoundRect(x + 10, y + 10, 38, 38, 8, apps[index].accent);
    tft.setTextColor(COL_BLACK);
    tft.setTextSize((strlen(apps[index].symbol) > 1) ? 1 : 2);

    int16_t x1, y1;
    uint16_t tw, th;
    tft.getTextBounds(apps[index].symbol, 0, 0, &x1, &y1, &tw, &th);
    tft.setCursor(x + 10 + (38 - (int)tw) / 2,
                  y + 10 + (38 - (int)th) / 2);
    tft.print(apps[index].symbol);

    tft.setTextSize(1);
    tft.setTextColor(selected ? COL_CYAN : COL_WHITE);
    tft.setCursor(x + 57, y + 14);
    tft.print(apps[index].name);

    tft.setTextColor(COL_MUTED);
    tft.setCursor(x + 57, y + 30);
    tft.print(apps[index].subtitle);

    if (selected)
    {
        tft.fillCircle(x + w - 12, y + h - 12, 3, COL_CYAN);
    }
}

void drawLauncher()
{
    tft.fillScreen(COL_BG);
    drawHeader("HOME");

    // Four launcher tiles in a 2 x 2 grid.
    drawAppCard(0, 12, 47, 145, 72);
    drawAppCard(1, 163, 47, 145, 72);
    drawAppCard(2, 12, 127, 145, 72);
    drawAppCard(3, 163, 127, 145, 72);

    drawFooter("D-PAD: MOVE   SELECT: OPEN   BACK: EXIT");
}

void moveSelection(int dx, int dy)
{
    int col = selectedApp % 2;
    int row = selectedApp / 2;

    col += dx;
    row += dy;

    if (col < 0)
        col = 1;
    if (col > 1)
        col = 0;
    if (row < 0)
        row = 1;
    if (row > 1)
        row = 0;

    selectedApp = row * 2 + col;
    drawLauncher();

    Serial.print("APP SELECTED: ");
    Serial.println(apps[selectedApp].name);
}

// =====================================================
// APP PLACEHOLDER SCREEN
// =====================================================

void drawAppScreen()
{
    tft.fillScreen(COL_BG);
    drawHeader(apps[openedApp].name);

    tft.fillRoundRect(24, 57, 272, 112, 12, COL_PANEL);
    tft.drawRoundRect(24, 57, 272, 112, 12, apps[openedApp].accent);

    drawCenteredText(apps[openedApp].name, 82, 2, COL_WHITE);
    drawCenteredText("COMING SOON", 116, 2, apps[openedApp].accent);
    drawCenteredText("This app will be ported", 145, 1, COL_MUTED);

    drawFooter("BACK: RETURN TO LAUNCHER");
}

void openSelectedApp()
{
    openedApp = selectedApp;
    currentScreen = SCREEN_APP;
    drawAppScreen();

    Serial.print("APP OPENED: ");
    Serial.println(apps[openedApp].name);
}

void returnToLauncher()
{
    currentScreen = SCREEN_LAUNCHER;
    drawLauncher();
    Serial.println("UI: returned to launcher");
}

// =====================================================
// INPUT ACTIONS
// =====================================================

void handleButtonAction(const char *name)
{
    Serial.print("{\"event\":\"pressed\",\"button\":\"");
    Serial.print(name);
    Serial.println("\"}");

    // HOME always returns to launcher.
    if (strcmp(name, "HOME") == 0)
    {
        returnToLauncher();
        return;
    }

    // MENU opens the launcher from any screen.
    if (strcmp(name, "MENU") == 0)
    {
        returnToLauncher();
        Serial.println("UI: MENU");
        return;
    }

    if (currentScreen == SCREEN_BOOT)
    {
        currentScreen = SCREEN_LAUNCHER;
        drawLauncher();
        Serial.println("UI: boot skipped");
        return;
    }

    if (currentScreen == SCREEN_APP)
    {
        if (strcmp(name, "BACK") == 0)
        {
            returnToLauncher();
        }
        return;
    }

    if (currentScreen != SCREEN_LAUNCHER)
        return;

    if (strcmp(name, "UP") == 0)
    {
        moveSelection(0, -1);
    }
    else if (strcmp(name, "DOWN") == 0)
    {
        moveSelection(0, 1);
    }
    else if (strcmp(name, "LEFT") == 0)
    {
        moveSelection(-1, 0);
    }
    else if (strcmp(name, "RIGHT") == 0)
    {
        moveSelection(1, 0);
    }
    else if (strcmp(name, "SELECT") == 0)
    {
        openSelectedApp();
    }
    else if (strcmp(name, "BACK") == 0)
    {
        // On the home screen BACK has no parent screen.
        Serial.println("UI: already at launcher");
    }
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

                    if (strcmp(button.name, "SELECT") != 0)
                    {
                        handleButtonAction(button.name);
                    }
                }
                else if (strcmp(button.name, "SELECT") == 0)
                {
                    if (!button.longPressHandled)
                    {
                        handleButtonAction("SELECT");
                    }
                }
            }
        }

        // Long SELECT = HOME. It is detected while held.
        if (strcmp(button.name, "SELECT") == 0 &&
            button.stableState == LOW &&
            !button.longPressHandled &&
            now - button.pressStartTime >= SELECT_LONG_PRESS_MS)
        {
            button.longPressHandled = true;
            handleButtonAction("HOME");
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

    Serial.println();
    Serial.println("==============================");
    Serial.println(" GITHUB BADGE ESP32");
    Serial.println(" MILESTONE 1: BOOT + LAUNCHER");
    Serial.println("==============================");
    Serial.println("BACK = GPIO 16");
    Serial.println("MENU = BOOT / GPIO 0");
    Serial.println("HOME = hold SELECT for 800 ms");

    startBoot();
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
    unsigned long now = millis();

    scanButtons(now);

    if (currentScreen == SCREEN_BOOT)
    {
        updateBoot();
    }
}
