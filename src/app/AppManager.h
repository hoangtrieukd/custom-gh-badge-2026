#pragma once

#include <stddef.h>
#include "App.h"
#include "../ui/Launcher.h"

constexpr size_t kMaxApps = kLauncherSlots;

// Owns the app registry, the launcher and the active-app lifecycle.
// Apps are registered by pointer and must outlive the manager
// (statically allocated apps are the intended use).
class AppManager
{
public:
    explicit AppManager(AppContext &context);

    bool registerApp(App *app);
    size_t appCount() const { return count_; }

    void showLauncher();
    bool launch(size_t index);
    void exitToLauncher();
    bool isAppActive() const { return active_ != nullptr; }

    void handleButton(AppButton button);
    void update();

private:
    void serviceRequests();

    AppContext &context_;
    App *apps_[kMaxApps];
    size_t count_ = 0;
    App *active_ = nullptr;
    Launcher launcher_;
};