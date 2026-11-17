#pragma once

#include <stdint.h>
#include <Adafruit_ST7789.h>

class BootScreen
{
public:
    void start(Adafruit_ST7789 &tft, unsigned long now);

    // Animates the progress bar. Returns true once the boot time has elapsed.
    bool update(Adafruit_ST7789 &tft, unsigned long now);

private:
    void draw(Adafruit_ST7789 &tft) const;

    unsigned long startedAt_ = 0;
    unsigned long lastFrameAt_ = 0;
    uint8_t frame_ = 0;
};