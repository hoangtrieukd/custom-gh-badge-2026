#pragma once

#include "PlaceholderApp.h"

class SettingsApp : public PlaceholderApp
{
public:
    SettingsApp()
        : PlaceholderApp(AppInfo{"SETTINGS", "Device settings", "S", COL_ORANGE}) {}
};