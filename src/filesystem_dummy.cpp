// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "filesystem.h"
#include "filesystem_dummy.h"

namespace circa {

FakeFileSystem* g_currentFakeFilesystem = NULL;

bool dummy_file_exists(const char* filename)
{
    return g_currentFakeFilesystem->_files.find(std::string(filename))
        != g_currentFakeFilesystem->_files.end();
}

void dummy_read_text_file(const char* filename, ReadFileCallback receiveFile, void* context)
{
    if (!dummy_file_exists(filename))
        receiveFile(context, NULL, "File not found");
    else
        receiveFile(context,
                g_currentFakeFilesystem->_files[std::string(filename)].contents.c_str(),
                NULL);
}

void dummy_write_text_file(const char* filename, const char* contents)
{
    g_currentFakeFilesystem->_files[std::string(filename)].contents = contents;
}

time_t dummy_get_modified_time(const char* filename)
{
    if (!dummy_file_exists(filename))
        return 0;
    else
        return g_currentFakeFilesystem->_files[std::string(filename)].last_modified;
}

static void dummy_read_directory(const char* dirname, circa::ReadDirectoryCallback callback,
       void* context)
{
    // Fake filesystem only has one directory. If the user requested the contents
    // of '/' or '.' then give them every file, otherwise give them nothing.
    if (!((strcmp(dirname, "/") == 0) || (strcmp(dirname, ".") == 0))) {
        callback(context, NULL, "directory doesn't exist");
        return;
    }

    std::map<std::string, FakeFileSystem::File>::const_iterator it;
    for (it = g_currentFakeFilesystem->_files.begin();
            it != g_currentFakeFilesystem->_files.end(); ++it) {
        bool callbackResult = callback(context, it->first.c_str(), NULL);
        if (!callbackResult)
            break;
    }
}

FakeFileSystem::FakeFileSystem()
{
    get_current_storage_interface(&_previousInterface);
    _previousFakeFilesystem = g_currentFakeFilesystem;

    StorageInterface interface;
    interface.readTextFile = dummy_read_text_file;
    interface.writeTextFile = dummy_write_text_file;
    interface.getModifiedTime = dummy_get_modified_time;
    interface.fileExists = dummy_file_exists;
    interface.readDirectory = dummy_read_directory;

    install_storage_interface(&interface);
    g_currentFakeFilesystem = this;
}

FakeFileSystem::~FakeFileSystem()
{
    g_currentFakeFilesystem = _previousFakeFilesystem;
    install_storage_interface(&_previousInterface);
}

std::string& FakeFileSystem::operator [] (std::string const& filename)
{
    return _files[filename].contents;
}
time_t& FakeFileSystem::last_modified(std::string const& filename)
{
    return _files[filename].last_modified;
}

void FakeFileSystem::set(std::string const& filename, std::string const& contents)
{
    _files[filename].contents = contents;
    _files[filename].last_modified++;
}

} // namespace circa
