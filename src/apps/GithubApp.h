#pragma once

#include "PlaceholderApp.h"

class GithubApp : public PlaceholderApp
{
public:
    GithubApp()
        : PlaceholderApp(AppInfo{"GITHUB", "GitHub experience", "GH", COL_GREEN}) {}
};