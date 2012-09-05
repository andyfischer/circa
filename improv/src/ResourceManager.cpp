// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "Common.h"

#include "circa/circa.h"
#include "circa/file.h"

#include "ResourceManager.h"

void ResourceManager::loadAsText(const char* filename, caValue* result)
{
    circa_read_file(filename, result);
}
