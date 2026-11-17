#pragma once

#include <stddef.h>
#include "../app/App.h"

// The launcher grid is 2 x 2, so it shows at most 4 apps for now.
constexpr size_t kLauncherSlots = 4;

class Launcher
{
public:
    void draw(AppContext &context, App *const *apps, size_t count) const;

    // Handles D-pad movement. Returns true if the button was a D-pad button.
    bool move(AppContext &context, App *const *apps, size_t count,
              AppButton button);

    size_t selected() const { return selected_; }

private:
    size_t selected_ = 0;
};