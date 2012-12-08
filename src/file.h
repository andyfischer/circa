// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// File reading
void read_text_file(const char* filename, caValue* contentsOut);
int file_get_mtime(const char* filename);

// File writing
void write_text_file(const char* filename, const char* contents);

// Path related functions
bool is_absolute_path(caValue* path);
void get_directory_for_filename(caValue* filename, caValue* result);
void get_parent_directory(caValue* filename, caValue* result);
void get_path_relative_to_source(caBlock* relativeTo, caValue* relPath, caValue* result);
void join_path(caValue* left, caValue* right);
void get_just_filename_for_path(caValue* path, caValue* filenameOut);

} // namespace circa
