// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "debug.h"
#include "fakefs.h"
#include "tagged_value.h"

namespace circa {

bool g_fakeSystemEnabled = false;
std::map<std::string, std::string> g_fileData;
std::map<std::string, int> g_fileMtime;

bool fakefs_enabled()
{
    return g_fakeSystemEnabled;
}

void enable_fakefs()
{
    g_fakeSystemEnabled = true;
}

void disable_fakefs()
{
    g_fakeSystemEnabled = false;
    g_fileData.clear();
    g_fileMtime.clear();
}

void fakefs_read_file(const char* filename, caValue* contentsOut)
{
    std::map<std::string, std::string>::const_iterator it =
        g_fileData.find(filename);

    if (it == g_fileData.end()) {
        set_null(contentsOut);
        return;
    }

    set_string(contentsOut, it->second.c_str());
}

bool fakefs_file_exists(const char* filename)
{
    std::map<std::string, std::string>::const_iterator it =
        g_fileData.find(filename);

    return it != g_fileData.end();
}

int fakefs_get_mtime(const char* filename)
{
    std::map<std::string, int>::const_iterator it =
        g_fileMtime.find(filename);

    if (it == g_fileMtime.end())
        return 0;

    return it->second;
}

FakeFilesystem::FakeFilesystem()
{
    if (fakefs_enabled())
        internal_error("Fake filesystem is already enabled");

    enable_fakefs();
}

FakeFilesystem::~FakeFilesystem()
{
    disable_fakefs();
}

void 
FakeFilesystem::set(std::string filename, std::string data)
{
    g_fileData[filename] = data;
}

void
FakeFilesystem::set_mtime(std::string filename, int time)
{
    if (!fakefs_file_exists(filename.c_str()))
        internal_error("called set_mtime for a file that doesn't exist");

    g_fileMtime[filename] = time;
}

} // namespace circa
