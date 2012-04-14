// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <fstream>

#include "string_type.h"

#include "term.h"
#include "file_utils.h"

using namespace circa;

bool circa_is_absolute_path(caValue* path)
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
    
void circa_get_directory_for_filename(caValue* filename, caValue* result)
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

void circa_get_path_relative_to_source(caTerm* relativeTo, caValue* relPath, caValue* result)
{
    // Don't modify a blank path
    if (string_eq(relPath,"")) {
        set_string(result, "");
        return;
    }

    if (((Term*)relativeTo)->owningBranch == NULL) {
        copy(relPath, result);
        return;
    }

    // Don't modify absolute paths
    if (circa_is_absolute_path(relPath)) {
        copy(relPath, result);
        return;
    }

    std::string scriptLocation = get_source_file_location(((Term*)relativeTo)->owningBranch);

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

void circa_join_path(caValue* left, caValue* right)
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

void circa_write_text_file(const char* filename, const char* contents)
{
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary);
    file << contents;
    file.close();
}
