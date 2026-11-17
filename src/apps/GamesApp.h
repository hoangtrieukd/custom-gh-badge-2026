#pragma once

#include "PlaceholderApp.h"

class GamesApp : public PlaceholderApp
{
public:
    GamesApp()
        : PlaceholderApp(AppInfo{"GAMES", "Play badge games", "G", COL_PINK}) {}
};