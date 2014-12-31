// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "debug.h"
#include "file.h"
#include "file_source.h"
#include "hashtable.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "names.h"
#include "string.h"
#include "tagged_value.h"
#include "world.h"
#include "ext/read_tar.h"

namespace circa {

// FileSource values come in these flavors:
//
// Flat in-memory map:
//   map : Hashtable(filename : String -> [version : int, contents : String])
//
// Filesystem backed:
//   [:filesystem, rootDir : String]
//
// Tarball backed:
//   [:tarball, contents : Blob]


static bool file_source_is_map(Value* file_source)
{
    return is_hashtable(file_source);
}

static bool file_source_is_filesystem_backed(Value* file_source)
{
    return is_list(file_source)
        && (list_length(file_source) >= 1)
        && (is_symbol(list_get(file_source, 0)))
        && (as_symbol(list_get(file_source, 0)) == sym_Filesystem);
}

static bool file_source_is_tarball_backed(Value* file_source)
{
    return is_list(file_source)
        && (list_length(file_source) >= 1)
        && (is_symbol(list_get(file_source, 0)))
        && (as_symbol(list_get(file_source, 0)) == sym_Tarball);
}

void file_source_read_file(Value* file_source, Value* name, Value* contents)
{
    if (file_source_is_map(file_source)) {
        Value* entry = hashtable_get(file_source, name);
        if (entry == NULL)
            set_null(contents);
        else
            copy(list_get(entry, 1), contents);
        return;
    }

    else if (file_source_is_filesystem_backed(file_source)) {
        Value fullPath;
        Value* rootDir = list_get(file_source, 1);
        copy(rootDir, &fullPath);
        join_path(&fullPath, name);
        read_text_file(as_cstring(&fullPath), contents);
        return;
    }

    else if (file_source_is_tarball_backed(file_source)) {
        Value* tarball = list_get(file_source, 1);
        tar_read_file(tarball, as_cstring(name), contents);
        return;
    }
    internal_error("file_source_read_file: file_source type not recognized");
}

bool file_source_does_file_exist(Value* file_source, Value* name)
{
    if (file_source_is_map(file_source)) {
        return hashtable_get(file_source, name) != NULL;
    }
    else if (file_source_is_filesystem_backed(file_source)) {
        Value fullPath;
        Value* rootDir = list_get(file_source, 1);
        copy(rootDir, &fullPath);
        join_path(&fullPath, name);
        return file_exists(as_cstring(&fullPath));
    }
    else if (file_source_is_tarball_backed(file_source)) {
        Value* tarball = list_get(file_source, 1);
        return tar_file_exists(tarball, as_cstring(name));
    }
    internal_error("file_source_does_file_exist: file_source type not recognized");
    return false;
}

int file_source_get_file_version(Value* file_source, Value* name)
{
    if (file_source_is_map(file_source)) {
        Value* entry = hashtable_get(file_source, name);
        if (entry == NULL)
            return 0;

        return as_int(list_get(entry, 0));
    }
    else if (file_source_is_filesystem_backed(file_source)) {
        Value fullPath;
        Value* rootDir = list_get(file_source, 1);
        copy(rootDir, &fullPath);
        join_path(&fullPath, name);
        return file_get_mtime(as_cstring(&fullPath));
    }
    else if (file_source_is_tarball_backed(file_source)) {
        // Currently no version support for tarballs.
        return 1;
    }
    return 0;
    internal_error("file_source_get_file_version: file_source type not recognized");
}

void file_source_create_using_filesystem(Value* file_source, const char* rootDir)
{
    set_list(file_source, 2);
    set_symbol(list_get(file_source, 0), sym_Filesystem);
    set_string(list_get(file_source, 1), rootDir);
}

void file_source_create_from_tarball(Value* file_source, Value* blob)
{
    set_list(file_source, 2);
    set_symbol(list_get(file_source, 0), sym_Tarball);
    set_value(list_get(file_source, 1), blob);
}

CIRCA_EXPORT void circa_read_file(caWorld* world, const char* filename, Value* contentsOut)
{
    Value name;
    set_string(&name, filename);

    const char* builtinFile = find_builtin_file(filename);
    if (builtinFile != NULL) {
        set_string(contentsOut, builtinFile);
        return;
    }

    Value* fileSources = &world->fileSources;
    for (int i=list_length(fileSources) - 1; i >= 0; i--) {
        Value* file_source = list_get(fileSources, i);
        file_source_read_file(file_source, &name, contentsOut);
        if (!is_null(contentsOut))
            return;
    }

    set_null(contentsOut);
}

CIRCA_EXPORT void circa_read_file_with_stack(caStack* stack, const char* filename, Value* contentsOut)
{
    return ::circa_read_file(stack->world, filename, contentsOut);
}

CIRCA_EXPORT bool circa_file_exists(caWorld* world, const char* filename)
{
    Value name;
    set_string(&name, filename);

    const char* builtinFile = find_builtin_file(filename);
    if (builtinFile != NULL)
        return true;

    Value* fileSources = &world->fileSources;
    for (int i=list_length(fileSources) - 1; i >= 0; i--) {
        Value* file_source = list_get(fileSources, i);
        bool exists = file_source_does_file_exist(file_source, &name);
        if (exists)
            return true;
    }

    return false;
}

CIRCA_EXPORT int circa_file_get_version(caWorld* world, const char* filename)
{
    Value name;
    set_string(&name, filename);

    const char* builtinFile = find_builtin_file(filename);
    if (builtinFile != NULL)
        return 0;

    Value* fileSources = &world->fileSources;
    for (int i=list_length(fileSources) - 1; i >= 0; i--) {
        Value* file_source = list_get(fileSources, i);
        if (!file_source_does_file_exist(file_source, &name))
            continue;

        return file_source_get_file_version(file_source, &name);
    }

    return 0;
}

} // namespace circa

