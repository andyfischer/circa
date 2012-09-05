// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

struct ResourceManager;

bool load_shaders(ResourceManager* resources, const char* baseFilename, Program* programStruct);
