// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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

bool file_exists(std::string const& filename)
{
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
    
} // namespace circa
