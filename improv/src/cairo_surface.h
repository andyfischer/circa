// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cairo/cairo.h>

#pragma once

void set_cairo_context(caValue* value, cairo_t* context);

void cairo_native_patch(caNativePatch* module);
