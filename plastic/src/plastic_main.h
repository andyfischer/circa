// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "plastic_common_headers.h"

std::string find_runtime_file();
std::string find_asset_file(std::string const& filename);

bool reload_runtime();
int plastic_main(std::vector<std::string> args);
