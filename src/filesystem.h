// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

// filesystem.h
//
// Defines an interface for accessing persistent storage. This interface can
// either make use of the real OS filesystem (implemented inside
// filesystem_posix.cpp), or it can use a dummy filesystem that only exists
// in memory (filesystem_dummy.cpp). The latter is used for testing.
//
// To turn on the real filesystem interface, call 
// circa_use_default_filesystem_interface(). To use the dummy filesystem (for
// testing), create a FakeFileSystem object.

namespace circa {

std::string get_directory_for_filename(std::string const& filename);
bool is_absolute_path(std::string const& path);
std::string get_absolute_path(std::string const& path);

// Callback used in read_text_file(). If the file is read successfully, then
// 'contents' will contain its full contents. If there is an error, 'contents'
// will be NULL and 'error' will have a human-readable description of the error.
typedef void (*ReadFileCallback)(void* context, const char* contents, const char* error);

void read_text_file(const char* filename, ReadFileCallback callback, void* context);

// Read the file as a text file, and write the entire contents to 'contents' as a
// string. If there are any problems, and 'error' is non-NULL, then an error message is
// written to 'error'. (The caller can ignore errors by passing NULL for 'error').
void read_text_file_to_value(const char* filename, caValue* contents, caValue* error);

// Read the file as a text file, return contents as a std::string.
std::string read_text_file_as_str(const char* filename);

void write_text_file(const char* filename, const char* contents);
time_t get_modified_time(const char* filename);
bool file_exists(const char* filename);

// Callback used in read_directory(). The function should return a boolean
// indicating whether it should continue reading the directory.
typedef bool (*ReadDirectoryCallback)(void* context, const char* filename, const char* error);

void read_directory(const char* dirname, ReadDirectoryCallback callback,
    void* context);

void read_directory_as_list(const char* dirname, List* result);

// Storage interface structures:

typedef void (*ReadTextFile)(const char* filename, ReadFileCallback callback, void* context);
typedef void (*WriteTextFile)(const char* filename, const char* contents);
typedef time_t (*GetModifiedTime)(const char* filename);
typedef bool (*FileExists)(const char* filename);
typedef void (*ReadDirectory)(const char* filename, ReadDirectoryCallback callback, void* context);

struct StorageInterface {
    ReadTextFile readTextFile;
    WriteTextFile writeTextFile;
    GetModifiedTime getModifiedTime;
    FileExists fileExists;
    ReadDirectory readDirectory;
};

// Install the provided storage interface
void install_storage_interface(StorageInterface* interface);

// Copy the current storage interface to the given argument
void get_current_storage_interface(StorageInterface* interface);

void join_path(String* left, String* right);

} // namespace circa

extern "C" {
    void circa_use_standard_filesystem();
}
