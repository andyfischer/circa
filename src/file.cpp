// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include <fstream>
#include <sys/stat.h>

#include "fakefs.h"
#include "list.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

int file_get_mtime(const char* filename)
{
    if (fakefs_enabled())
        return fakefs_get_mtime(filename);

    struct stat s;
    s.st_mtime = 0;

    stat(filename, &s);
    return s.st_mtime;
}

bool is_absolute_path(caValue* path)
{
    // TODO: This function is bad, need to use an existing library for dealing
    // with paths.
    
    int len = string_length(path);

    if (len >= 1 && string_get(path, 0) == '/')
        return true;
    if (len >= 2 && string_get(path, 1) == ':')
        return true;
    return false;
}
    
void get_directory_for_filename(caValue* filename, caValue* result)
{
    // TODO: This function is bad, need to use an existing library for dealing
    // with paths.
    int last_slash = string_find_char_from_end(filename, '/');

    if (last_slash == -1) {
        set_string(result, ".");
        return;
    }

    if (last_slash == 0) {
        set_string(result, "/");
        return;
    }

    circa_set_string_size(result, as_cstring(filename), last_slash);
}

void get_path_relative_to_source(caBranch* relativeTo, caValue* relPath, caValue* result)
{
    // Don't modify a blank path
    if (string_eq(relPath,"")) {
        set_string(result, "");
        return;
    }

    if (relativeTo == NULL) {
        copy(relPath, result);
        return;
    }

    // Don't modify absolute paths
    if (is_absolute_path(relPath)) {
        copy(relPath, result);
        return;
    }

    std::string scriptLocation = get_source_file_location((Branch*) relativeTo);

    if (scriptLocation == "" || scriptLocation == ".") {
        copy(relPath, result);
        return;
    }

    set_string(result, scriptLocation.c_str());
    string_append(result, "/");
    string_append(result, relPath);
}

static bool is_path_seperator(char c)
{
    // Not UTF safe.
    return c == '/' || c == '\\';
}

void join_path(caValue* left, caValue* right)
{
    const char* leftStr = as_cstring(left);
    const char* rightStr = as_cstring(right);
    int left_len = strlen(leftStr);
    int right_len = strlen(leftStr);

    int seperatorCount = 0;
    if (left_len > 0 && is_path_seperator(leftStr[left_len-1]))
        seperatorCount++;

    if (right_len > 0 && is_path_seperator(rightStr[0]))
        seperatorCount++;

    if (seperatorCount == 2)
        string_resize(left, left_len - 1);
    else if (seperatorCount == 0)
        string_append(left, "/");

    string_append(left, right);
}

void get_just_filename_for_path(caValue* path, caValue* filenameOut)
{
    int start = string_length(path) - 1;
    while (start > 0 && !is_path_seperator(string_get(path, start - 1)))
        start--;

    string_slice(path, start, -1, filenameOut);
}

void write_text_file(const char* filename, const char* contents)
{
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary);
    file << contents;
    file.close();
}

} // namespace circa

using namespace circa;

extern "C" {

struct CachedFile {
    char* filename;
    circa::Value contents;
    bool needs_fread;
    int version;
    int last_known_mtime;
};

std::map<std::string, CachedFile*> g_fileCache;

static CachedFile* get_file_entry(const char* filename)
{
    std::map<std::string, CachedFile*>::const_iterator it;
    it = g_fileCache.find(filename);
    if (it == g_fileCache.end())
        return NULL;

    return it->second;
}

static CachedFile* create_file_entry(const char* filename)
{
    CachedFile* entry = get_file_entry(filename);
    if (entry != NULL)
        return entry;

    // Create a new entry
    entry = (CachedFile*) malloc(sizeof(*entry));
    entry->filename = circa_strdup(filename);
    initialize_null(&entry->contents);
    entry->needs_fread = false;
    entry->version = 0;
    entry->last_known_mtime = 0;
    g_fileCache[filename] = entry;
    return entry;
}

static void update_version_from_mtime(CachedFile* entry)
{
    unsigned mtime = file_get_mtime(entry->filename);

    if (entry->last_known_mtime != mtime) {
        entry->last_known_mtime = mtime;
        entry->version++;
        entry->needs_fread = true;
    }
}

void circa_read_file(const char* filename, caValue* contentsOut)
{
    if (fakefs_enabled())
        return fakefs_read_file(filename, contentsOut);
    
    CachedFile* entry = create_file_entry(filename);
    update_version_from_mtime(entry);

    if (!entry->needs_fread)
        copy(&entry->contents, contentsOut);

    // Read the data
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        set_null(contentsOut);
        return;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    rewind(fp);

    char* contentsData = string_initialize(&entry->contents, file_size + 1);
    size_t bytesRead = fread(contentsData, 1, file_size, fp);

    contentsData[bytesRead] = 0;
    entry->needs_fread = false;

    copy(&entry->contents, contentsOut);

    log_start(0, "read_file");
    log_arg("filename", filename);
    log_arg("file_size", file_size);
    log_arg("bytesRead", bytesRead);
    log_arg("contents", entry->contents);
    log_finish();
}

bool circa_file_exists(const char* filename)
{
    if (fakefs_enabled())
        return fakefs_file_exists(filename);

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return false;

    fclose(fp);
    return true;
}

int circa_file_get_version(const char* filename)
{
    CachedFile* entry = create_file_entry(filename);
    update_version_from_mtime(entry);
    return entry->version;
}

} // extern "C"
