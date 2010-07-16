// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

// Functions for accessing persistent storage

namespace circa {

std::string get_path_relative_to_source(Term* relativeTo, std::string const& path);
std::string get_directory_for_filename(std::string const& filename);
std::string get_absolute_path(std::string const& path);

namespace storage {

typedef std::string (*ReadTextFile)(std::string const& filename);
typedef void (*WriteTextFile)(std::string const& filename, std::string const& contents);
typedef time_t (*GetModifiedTime)(std::string const& filename);
typedef bool (*FileExists)(std::string const& filename);

std::string read_text_file(std::string const& filename);
void write_text_file(std::string const& filename, std::string const& contents);
time_t get_modified_time(std::string const& filename);
bool file_exists(std::string const& filename);

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
