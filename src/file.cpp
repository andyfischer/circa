// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include <fstream>
#include <sys/stat.h>

#include "list.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

static bool is_path_seperator(char c)
{
    // Not UTF safe.
    return c == '/' || c == '\\';
}

int file_get_mtime(const char* filename)
{
    struct stat s;
    s.st_mtime = 0;

    if (stat(filename, &s) != 0)
        return 0;

    return (int) s.st_mtime;
}

bool file_exists(const char* filename)
{
    struct stat s;

    if (stat(filename, &s) != 0)
        return false;

    return S_ISREG(s.st_mode);
}

bool is_absolute_path(Value* path)
{
    int len = string_length(path);

    if (len >= 1 && string_get(path, 0) == '/')
        return true;
    if (len >= 2 && string_get(path, 1) == ':')
        return true;
    return false;
}
    
void get_directory_for_filename(Value* filename, Value* result)
{
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

CIRCA_EXPORT void circa_get_directory_for_filename(Value* filename, Value* result)
{
    get_directory_for_filename(filename, result);
}

void get_parent_directory(Value* filename, Value* result)
{
    int end = string_length(filename);

    // Advance past trailing seperators.
    while (end > 0 && is_path_seperator(string_get(filename, end - 1)))
        end--;

    bool foundSep = false;

    while (end > 0) {
        while (end > 0 && is_path_seperator(string_get(filename, end - 1))) {
            foundSep = true;
            end--;
        }

        if (foundSep)
            break;

        end--;
    }

    if (end == 0) {
        if (is_absolute_path(filename))
            set_string(result, "/");
        else
            set_string(result, ".");
    } else
        string_slice(filename, 0, end, result);
}

CIRCA_EXPORT void circa_get_parent_directory(Value* filename, Value* result)
{
    get_parent_directory(filename, result);
}

void get_path_relative_to_source(caBlock* relativeTo, Value* relPath, Value* result)
{
    if (relativeTo == NULL) {
        copy(relPath, result);
        return;
    }

    // Don't modify absolute paths
    if (is_absolute_path(relPath)) {
        copy(relPath, result);
        return;
    }

    Value scriptLocation;
    get_source_file_location(relativeTo, &scriptLocation);

    if (string_equals(&scriptLocation, "") || string_equals(&scriptLocation, ".")) {
        set_value(result, relPath);
        return;
    }

    set_value(result, &scriptLocation);

    if (!string_equals(relPath,"")) {
        string_append(result, "/");
        string_append(result, relPath);
    }
}

void join_path(Value* left, Value* right)
{
    const char* leftStr = as_cstring(left);
    const char* rightStr = as_cstring(right);
    int left_len = (int) strlen(leftStr);
    int right_len = (int) strlen(leftStr);

    if (string_equals(left, "") || string_equals(left, ".")) {
        copy(right, left);
        return;
    }

    if (string_equals(right, ""))
        return;

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

void get_just_filename_for_path(Value* path, Value* filenameOut)
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

void read_text_file(const char* filename, Value* contentsOut)
{
    if (!file_exists(filename)) {
        set_null(contentsOut);
        return;
    }

    FILE* fp = fopen(filename, "r");

    if (fp == NULL) {
        // Unlikely to reach here since we just checked file_exists. But possible.
        set_null(contentsOut);
        return;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    rewind(fp);

    // Read raw data.
    touch(contentsOut);
    set_blob(contentsOut, int(file_size));

    size_t bytesRead = fread(as_blob(contentsOut), 1, file_size, fp);
    
    if (bytesRead < file_size)
        string_resize(contentsOut, (int) bytesRead);

    fclose(fp);
}

} // namespace "circa"
