// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "circa/circa.h"

extern caWorld* g_world;
extern caStack* g_mainStack;

// Initialize Circa world
void scripts_initialize();

void scripts_refresh();

void scripts_pre_message_send();
void scripts_post_message_send();
