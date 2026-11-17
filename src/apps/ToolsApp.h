#pragma once

#include "PlaceholderApp.h"

class ToolsApp : public PlaceholderApp
{
public:
    ToolsApp()
        : PlaceholderApp(AppInfo{"TOOLS", "Badge utilities", "T", COL_CYAN}) {}
};