// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

// Functions for accessing persistent storage

namespace circa {

std::string get_path_relative_to_source(Term* relativeTo, std::string const& path);
std::string get_directory_for_filename(std::string const& filename);
std::string get_absolute_path(std::string const& path);

namespace storage {

typedef void (*FileReceiveFunc)(void* context, const char* contents);
typedef void (*ReadTextFile)(const char* filename, FileReceiveFunc receiveFile, void* context);
typedef void (*WriteTextFile)(const char* filename, const char* contents);
typedef time_t (*GetModifiedTime)(const char* filename);
typedef bool (*FileExists)(const char* filename);

void read_text_file(const char* filename, FileReceiveFunc receiveFile, void* context);
std::string read_text_file_as_str(const char* filename);
void write_text_file(const char* filename, const char* contents);
time_t get_modified_time(const char* filename);
bool file_exists(const char* filename);

struct StorageInterface {
    ReadTextFile readTextFile;
    WriteTextFile writeTextFile;
    GetModifiedTime getModifiedTime;
    FileExists fileExists;
};

// Install the provided storage interface
void install_storage_interface(StorageInterface* interface);

// Copy the current storage interface to the given argument
void get_current_storage_interface(StorageInterface* interface);

// Install a builtin interface that just uses the filesystem in a standard way
void use_filesystem();

} // namespace storage
} // namespace circa
