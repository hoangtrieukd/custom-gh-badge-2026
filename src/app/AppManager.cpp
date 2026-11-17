#include "AppManager.h"

AppManager::AppManager(AppContext &context) : context_(context)
{
    for (size_t i = 0; i < kMaxApps; i++)
    {
        apps_[i] = nullptr;
    }
}

bool AppManager::registerApp(App *app)
{
    if (app == nullptr || count_ >= kMaxApps)
        return false;

    apps_[count_++] = app;
    return true;
}

void AppManager::showLauncher()
{
    launcher_.draw(context_, apps_, count_);
}

bool AppManager::launch(size_t index)
{
    if (index >= count_)
        return false;

    active_ = apps_[index];
    context_.exitRequested = false;
    context_.redrawRequested = false;

    context_.console.print("APP OPENED: ");
    context_.console.println(active_->name());

    active_->onEnter(context_);

    if (context_.exitRequested)
    {
        exitToLauncher();
        return true;
    }

    active_->draw(context_);
    context_.redrawRequested = false;
    return true;
}

void AppManager::exitToLauncher()
{
    if (active_ != nullptr)
    {
        App *leaving = active_;
        active_ = nullptr;
        leaving->onExit(context_);
    }

    context_.exitRequested = false;
    context_.redrawRequested = false;

    showLauncher();
    context_.console.println("UI: returned to launcher");
}

void AppManager::handleButton(AppButton button)
{
    // System buttons always return to the launcher, whatever the app does.
    if (button == AppButton::Menu || button == AppButton::Home)
    {
        exitToLauncher();
        return;
    }

    if (active_ != nullptr)
    {
        bool handled = active_->onButton(context_, button);
        if (!handled && button == AppButton::Back)
        {
            context_.requestExit();
        }
        serviceRequests();
        return;
    }

    // Launcher input
    if (launcher_.move(context_, apps_, count_, button))
        return;

    if (button == AppButton::Select)
    {
        launch(launcher_.selected());
    }
    else if (button == AppButton::Back)
    {
        // On the home screen BACK has no parent screen.
        context_.console.println("UI: already at launcher");
    }
}

void AppManager::update()
{
    if (active_ == nullptr)
        return;

    active_->update(context_);
    serviceRequests();
}

void AppManager::serviceRequests()
{
    if (active_ == nullptr)
        return;

    if (context_.exitRequested)
    {
        exitToLauncher();
        return;
    }

    if (context_.redrawRequested)
    {
        context_.redrawRequested = false;
        active_->draw(context_);
    }
}