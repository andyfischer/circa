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
// circ_use_default_filesystem_interface(). To use the dummy filesystem (for
// testing), create a FakeFileSystem object.

namespace circa {

// Callback used in read_directory(). The function should return a boolean
// indicating whether it should continue reading the directory.
typedef bool (*ReadDirectoryCallback)(void* context, const char* filename, const char* error);

void read_directory(const char* dirname, ReadDirectoryCallback callback,
    void* context);

void read_directory_as_list(const char* dirname, List* result);

} // namespace circa
