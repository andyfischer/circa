// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

bool circa_is_absolute_path(caValue* path);
void circa_get_directory_for_filename(caValue* filename, caValue* result);
void circa_get_path_relative_to_source(caTerm* relativeTo, caValue* relPath, caValue* result);
