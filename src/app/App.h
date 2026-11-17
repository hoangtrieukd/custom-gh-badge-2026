#pragma once

#include <Arduino.h>
#include <Adafruit_ST7789.h>

// Logical buttons delivered to the system. MENU and HOME are handled by
// the AppManager and are never delivered to an app.
enum class AppButton : uint8_t
{
    Up,
    Down,
    Left,
    Right,
    Select,
    Back,
    Menu,
    Home
};

inline const char *appButtonName(AppButton button)
{
    switch (button)
    {
    case AppButton::Up:
        return "UP";
    case AppButton::Down:
        return "DOWN";
    case AppButton::Left:
        return "LEFT";
    case AppButton::Right:
        return "RIGHT";
    case AppButton::Select:
        return "SELECT";
    case AppButton::Back:
        return "BACK";
    case AppButton::Menu:
        return "MENU";
    case AppButton::Home:
        return "HOME";
    }
    return "?";
}

// Launcher metadata for an app.
struct AppInfo
{
    const char *name;
    const char *subtitle;
    const char *symbol;
    uint16_t accent;
};

// Everything an app may touch. Apps never use globals or Serial directly.
class AppContext
{
public:
    AppContext(Adafruit_ST7789 &display, Print &out)
        : tft(display), console(out) {}

    Adafruit_ST7789 &tft;
    Print &console; // serial logging
    unsigned long now = 0;

    // Ask the AppManager to leave this app / redraw it after the
    // current callback returns.
    void requestExit() { exitRequested = true; }
    void requestRedraw() { redrawRequested = true; }

    bool exitRequested = false;
    bool redrawRequested = false;
};

class App
{
public:
    virtual ~App() = default;

    virtual const AppInfo &info() const = 0;
    const char *name() const { return info().name; }

    // Called once when the app is launched. Acquire resources here.
    virtual void onEnter(AppContext &context) = 0;

    // Draw the whole screen. Called after onEnter() and after requestRedraw().
    virtual void draw(AppContext &context) = 0;

    // Called every loop() iteration while the app is active.
    virtual void update(AppContext &) {}

    // Return true if the press was consumed. An unconsumed BACK exits the app.
    virtual bool onButton(AppContext &, AppButton) { return false; }

    // Called once when the app is left for any reason. Release resources here.
    virtual void onExit(AppContext &) {}
};