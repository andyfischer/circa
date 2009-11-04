// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <sys/stat.h>

#include "circa.h"

namespace circa {

FileIORedirector* FAKE_FILE_IO = NULL;

std::string read_text_file(std::string const& filename)
{
    if (FAKE_FILE_IO != NULL)
        return FAKE_FILE_IO->read_text_file(filename);

    std::ifstream file;
    file.open(filename.c_str(), std::ios::in);
    std::stringstream contents;
    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) {
        if (!firstLine)
            contents << "\n";
        contents << line;
        firstLine = false;
    }
    file.close();
    return contents.str();
}

void write_text_file(std::string const& filename, std::string const& contents)
{
    if (FAKE_FILE_IO != NULL) {
        FAKE_FILE_IO->write_text_file(filename, contents);
        return;
    }

    std::ofstream file;
    file.open(filename.c_str(), std::ios::out | std::ios::binary);
    file << contents;
    file.close();
}

time_t get_modified_time(std::string const& filename)
{
    if (FAKE_FILE_IO != NULL)
        return FAKE_FILE_IO->get_modified_time(filename);

    struct stat s;

    s.st_mtime = 0;

    stat(filename.c_str(), &s);

    return s.st_mtime;
}

std::string get_directory_for_filename(std::string const& filename)
{
    // Should probably use boost::path or some other library here.
    size_t last_slash = filename.find_last_of("/");

    if (last_slash == filename.npos)
        return ".";

    if (last_slash == 0)
        return "/";

    std::string result = filename.substr(0, last_slash);

    return result;
}

bool is_absolute_path(std::string const& path)
{
    if (path.length() >= 1 && path[0] == '/')
        return true;
    if (path.length() >= 2 && path[1] == ':')
        return true;
    return false;
}

std::string get_path_relative_to_source(Term* relativeTo, std::string const& path)
{
    if (relativeTo->owningBranch == NULL)
        return path;

    // Don't modify absolute paths
    if (is_absolute_path(path))
        return path;

    std::string scriptLocation = get_source_file_location(*relativeTo->owningBranch);

    if (scriptLocation == "")
        return path;

    return scriptLocation + "/" + path;
}

std::string get_absolute_path(std::string const& path)
{
    if (is_absolute_path(path))
        return path;

    char buf[512];
    std::string cwd = getcwd(buf, 512);

    return cwd + "/" + path;
}

bool file_exists(std::string const& filename)
{
    if (FAKE_FILE_IO != NULL)
        return FAKE_FILE_IO->file_exists(filename);

    // This could also be replaced by boost
    FILE* fp = fopen(filename.c_str(), "r");
    if (fp) {
        // file exists
        fclose(fp);
        return true;
    } else {
        return false;
    }
}

void install_fake_file_io(FileIORedirector* fakeObject)
{
    FAKE_FILE_IO = fakeObject;
}

FakeFileSystem::FakeFileSystem() { install_fake_file_io(this); }
FakeFileSystem::~FakeFileSystem() { install_fake_file_io(NULL); }

std::string& FakeFileSystem::operator [] (std::string const& filename)
{
    return _files[filename].contents;
}

time_t& FakeFileSystem::last_modified(std::string const& filename)
{
    return _files[filename].last_modified;
}

std::string FakeFileSystem::read_text_file(std::string const& filename)
{
    return _files[filename].contents;
}

void FakeFileSystem::write_text_file(std::string const& filename, std::string const& contents)
{
    _files[filename].contents = contents;
}

time_t FakeFileSystem::get_modified_time(std::string const& filename)
{
    return _files[filename].last_modified;
}

bool FakeFileSystem::file_exists(std::string const& filename)
{
    return _files.find(filename) != _files.end();
}

} // namespace circa
