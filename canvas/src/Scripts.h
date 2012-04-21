
#pragma once

#include "circa/circa.h"

extern caWorld* g_world;
extern caStack* g_mainStack;

// Initialize Circa world
void scripts_initialize();

void scripts_refresh();

// Run the function on g_mainStack. Handles error checking.
bool scripts_run();
