// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// ResourceManager
//
// Loads asset files using string filenames.

#pragma once

#include "circa/circa.h"

struct ResourceManager
{
    virtual void loadAsText(const char* filename, caValue* result);
};
