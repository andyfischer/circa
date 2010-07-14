// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "plastic_common_headers.h"

extern int TARGET_FPS;

std::string find_runtime_file();
std::string find_asset_file(std::string const& filename);
bool evaluate_main_script();

// Logging:
void error(std::string const& msg);

bool reload_runtime();
int plastic_main(std::vector<std::string> args);
