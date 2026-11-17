#pragma once

#include "../app/App.h"
#include "../ui/Theme.h"

// Shared "COMING SOON" app used until the real badge apps are ported.
// It shows per-instance counters so input routing can be verified.
class PlaceholderApp : public App
{
public:
    explicit PlaceholderApp(const AppInfo &info);

    const AppInfo &info() const override { return info_; }

    void onEnter(AppContext &context) override;
    void draw(AppContext &context) override;
    bool onButton(AppContext &context, AppButton button) override;
    void onExit(AppContext &context) override;

private:
    void drawStatus(Adafruit_ST7789 &tft) const;

    AppInfo info_;
    uint16_t enterCount_ = 0;  // survives between launches (per instance)
    uint16_t buttonCount_ = 0; // reset on every launch
    AppButton lastButton_ = AppButton::Up;
    bool hasLastButton_ = false;
};