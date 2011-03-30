// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "storage.h"

namespace circa {

// Dummy class that implements StorageInterface. This class installs itself on construction
// and deletes itself afterwards. This class is only meant for unit tests.
struct FakeFileSystem
{
    struct File {
        std::string contents;
        time_t last_modified;

        File() : last_modified(0) {}
    };

    storage::StorageInterface _previousInterface;
    FakeFileSystem* _previousFakeFilesystem;

    std::map<std::string, File> _files;

    FakeFileSystem();
    ~FakeFileSystem();

    std::string& operator [] (std::string const& filename);
    time_t& last_modified(std::string const& filename);
};

} // namespace circa
