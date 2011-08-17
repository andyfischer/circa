// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "filesystem.h"
#include "filesystem_dummy.h"

namespace circa {

FakeFileSystem* g_currentFakeFilesystem = NULL;

namespace fakefilesystem_interface {

    bool file_exists(const char* filename)
    {
        return g_currentFakeFilesystem->_files.find(std::string(filename))
            != g_currentFakeFilesystem->_files.end();
    }

    void read_text_file(const char* filename, FileReceiveFunc receiveFile, void* context)
    {
        if (!file_exists(filename))
            receiveFile(context, NULL, "File not found");
        else
            receiveFile(context,
                    g_currentFakeFilesystem->_files[std::string(filename)].contents.c_str(),
                    NULL);
    }

    void write_text_file(const char* filename, const char* contents)
    {
        g_currentFakeFilesystem->_files[std::string(filename)].contents = contents;
    }

    time_t get_modified_time(const char* filename)
    {
        return g_currentFakeFilesystem->_files[std::string(filename)].last_modified;
    }
}

FakeFileSystem::FakeFileSystem()
{
    get_current_storage_interface(&_previousInterface);
    _previousFakeFilesystem = g_currentFakeFilesystem;

    StorageInterface interface;
    interface.readTextFile = fakefilesystem_interface::read_text_file;
    interface.writeTextFile = fakefilesystem_interface::write_text_file;
    interface.getModifiedTime = fakefilesystem_interface::get_modified_time;
    interface.fileExists = fakefilesystem_interface::file_exists;

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
