// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

// Functions for accessing persistent storage

namespace circa {

std::string get_path_relative_to_source(Term* relativeTo, std::string const& path);
std::string get_directory_for_filename(std::string const& filename);
std::string get_absolute_path(std::string const& path);

namespace storage {

std::string read_text_file(std::string const& filename);
void write_text_file(std::string const& filename, std::string const& contents);
time_t get_modified_time(std::string const& filename);
bool file_exists(std::string const& filename);

struct StorageInterface {
    virtual ~StorageInterface() {}
    virtual std::string read_text_file(std::string const& filename) = 0;
    virtual void write_text_file(std::string const& filename, std::string const& contents) = 0;
    virtual time_t get_modified_time(std::string const& filename) = 0;
    virtual bool file_exists(std::string const& filename) = 0;
};

// Install the provided storage interface
void install_storage_interface(StorageInterface* interface);

// Install a builtin interface that just uses the filesystem
void use_filesystem();

// Dummy class that implements StorageInterface. This class installs itself on construction
// and deletes itself afterwards.
struct FakeFileSystem : StorageInterface
{
    struct File {
        std::string contents;
        time_t last_modified;

        File() : last_modified(0) {}
    };

    StorageInterface *_previousInterface;

    std::map<std::string, File> _files;

    FakeFileSystem();
    ~FakeFileSystem();

    std::string& operator [] (std::string const& filename);
    time_t& last_modified(std::string const& filename);
    virtual std::string read_text_file(std::string const& filename);
    virtual void write_text_file(std::string const& filename, std::string const& contents);
    virtual time_t get_modified_time(std::string const& filename);
    virtual bool file_exists(std::string const& filename);
};

} // namespace storage
} // namespace circa
