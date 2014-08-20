// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// File reading
void read_text_file(const char* filename, Value* contentsOut);
int file_get_mtime(const char* filename);
bool file_exists(const char* filename);

// File writing
void write_text_file(const char* filename, const char* contents);

// Path related functions
bool is_absolute_path(Value* path);
void get_directory_for_filename(Value* filename, Value* result);
void get_parent_directory(Value* filename, Value* result);
void get_path_relative_to_source(caBlock* relativeTo, Value* relPath, Value* result);
void join_path(Value* left, Value* right);
void get_just_filename_for_path(Value* path, Value* filenameOut);

} // namespace circa
