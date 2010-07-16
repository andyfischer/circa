// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifdef WINDOWS
// for fopen()
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <sys/stat.h>

#include "branch.h"
#include "storage.h"
#include "term.h"

namespace circa {
namespace storage {

StorageInterface g_storageInterface;

std::string read_text_file(std::string const& filename)
{
    if (g_storageInterface.readTextFile == NULL)
        return "";
    return g_storageInterface.readTextFile(filename);
}

void write_text_file(std::string const& filename, std::string const& contents)
{
    if (g_storageInterface.writeTextFile == NULL)
        return;
    return g_storageInterface.writeTextFile(filename, contents);
}

time_t get_modified_time(std::string const& filename)
{
    if (filename == "")
        return 0;

    if (g_storageInterface.getModifiedTime == NULL)
        return 0;

    return g_storageInterface.getModifiedTime(filename);
}

bool file_exists(std::string const& filename)
{
    if (g_storageInterface.fileExists == NULL)
        return false;
    return g_storageInterface.fileExists(filename);
}

namespace filesystem_storage
{
    std::string read_text_file(std::string const& filename)
    {
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
        std::ofstream file;
        file.open(filename.c_str(), std::ios::out | std::ios::binary);
        file << contents;
        file.close();
    }

    time_t get_modified_time(std::string const& filename)
    {
        struct stat s;
        s.st_mtime = 0;

        stat(filename.c_str(), &s);

        return s.st_mtime;
    }

    bool file_exists(std::string const& filename)
    {
        // This could also be replaced by boost::path
        FILE* fp = fopen(filename.c_str(), "r");
        if (fp) {
            // file exists
            fclose(fp);
            return true;
        } else {
            return false;
        }
    }
};

void install_storage_interface(StorageInterface* interface)
{
    g_storageInterface = *interface;
}

void get_current_storage_interface(StorageInterface* interface)
{
    *interface = g_storageInterface;
}

void use_filesystem()
{
    g_storageInterface.readTextFile = filesystem_storage::read_text_file;
    g_storageInterface.writeTextFile = filesystem_storage::write_text_file;
    g_storageInterface.getModifiedTime = filesystem_storage::get_modified_time;
    g_storageInterface.fileExists = filesystem_storage::file_exists;
}

} // namespace storage

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
    // Don't modify a blank path
    if (path == "")
        return "";

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
#ifdef WINDOWS
    std::string cwd = _getcwd(buf, 512);
#else
    std::string cwd = getcwd(buf, 512);
#endif

    return cwd + "/" + path;
}
} // namespace circa
