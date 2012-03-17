// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "string_type.h"

#include "term.h"
#include "file_utils.h"

using namespace circa;

bool circa_is_absolute_path(caValue* path)
{
    // TODO: This function is terrible, need to use an existing library for dealing
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
    // TODO: This function is terrible, need to use an existing library for dealing
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

