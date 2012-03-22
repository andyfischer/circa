// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

bool circ_is_absolute_path(caValue* path);
void circ_get_directory_for_filename(caValue* filename, caValue* result);
void circ_get_path_relative_to_source(caTerm* relativeTo, caValue* relPath, caValue* result);

// Append 'right' to 'left' as a path.
void circ_join_path(caValue* left, caValue* right);

// Write a text file. This doesn't use the file-source framework.
void circ_write_text_file(const char* filename, const char* contents);
