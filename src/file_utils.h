// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

bool circa_is_absolute_path(caValue* path);
void circa_get_directory_for_filename(caValue* filename, caValue* result);
void circa_get_path_relative_to_source(caBranch* relativeTo, caValue* relPath, caValue* result);

// Append 'right' to 'left' as a path.
void circa_join_path(caValue* left, caValue* right);

// Write a text file. This doesn't use the file-source framework.
void circa_write_text_file(const char* filename, const char* contents);
