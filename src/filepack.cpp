// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

/*

Having problem with files:

  Currently in Nacl land, it's a little adhoc: we pass around the /http/ file prefix,
  and we rely on the fact that fread() is magically remapped to be a web access. This
  is fairly nacl-specific.

  Like most game engines, need a asset pack thing. Circa would load an asset pack,
  and then all fonts/images/sounds/etc come from the pack. This would highly optimize
  the Nacl pattern and it would make sense for other contexts too.

  Not to mention that Nacl fread() does not handle file modification!

  Having an asset layer that is backed by files is probably a good thing.

Implementation
  Definitely in the Circa layer
    Because module loading would be included in this.

  AssetPack (possible name) would be abstract
    Could be implemented by filesystem.
    Could be implemented by a packed file format.
    Could be implemented by wrapped fread (temporarily)

  Where would the AssetPack be stored?
    World?
    Stack?
    Frame context?

  In general.. it seems like we could have a Worldwide frame context, which every
  stack starts with. Then we could have ease and flexiblity when storing AssetPack
  as a context var.

  For 1st iteration, storing on World is probably fine.

  Must support multiple concurrent AssetPack.

  Have a order-dependent stack. When loading a file, start at the end and try every
  pack until we find one that successfully loads.

Definition of AssetPack interface:
  read(filename)
  exists(filename)
  get_version(filename)

Some design thoughts:

  - We could require that all of a filepack's files are known at once and iterable.
  - Or we could allow for filepack contents to be lazy. In this world, the filepack
    can only answer if a file exists when a name is provided. Iteration over all files
    is not always possible.
  - Middle ground: Iteration is possible but we support lazy loading.
  - Of course, iteration is time-dependent. If a file is added later then the list
    of iterated files would change.
  - So iterating over a filesystem-based pack would just be the same as a directory
    iteration. (assuming that there is no manifest file). This is fine as long as
    there is lazy loading.
  - Okay, lazy loading is a requirement, and iteration is allowed.

*/


#include "common_headers.h"

#include "debug.h"
#include "file.h"
#include "filepack.h"
#include "hashtable.h"
#include "interpreter.h"
#include "list.h"
#include "names.h"
#include "string.h"
#include "tagged_value.h"
#include "world.h"
#include "ext/read_tar.h"

namespace circa {

// FilePack values come in these flavors:
//
// Flat in-memory map:
//   map : Hashtable(filename : String -> [version : int, contents : String])
//
// Filesystem backed:
//   [:filesystem, rootDir : String]
//
// Tarball backed:
//   [:tarball, contents : Blob]


static bool filepack_is_map(caValue* filepack)
{
    return is_hashtable(filepack);
}

static bool filepack_is_filesystem_backed(caValue* filepack)
{
    return is_list(filepack)
        && (list_length(filepack) >= 1)
        && (is_symbol(list_get(filepack, 0)))
        && (as_symbol(list_get(filepack, 0)) == sym_Filesystem);
}

static bool filepack_is_tarball_backed(caValue* filepack)
{
    return is_list(filepack)
        && (list_length(filepack) >= 1)
        && (is_symbol(list_get(filepack, 0)))
        && (as_symbol(list_get(filepack, 0)) == sym_Tarball);
}

void filepack_read_file(caValue* filepack, caValue* name, caValue* contents)
{
    if (filepack_is_map(filepack)) {
        caValue* entry = hashtable_get(filepack, name);
        if (entry == NULL)
            set_null(contents);
        else
            copy(list_get(entry, 1), contents);
        return;
    }

    else if (filepack_is_filesystem_backed(filepack)) {
        Value fullPath;
        caValue* rootDir = list_get(filepack, 1);
        copy(rootDir, &fullPath);
        join_path(&fullPath, name);
        read_text_file(as_cstring(&fullPath), contents);
        return;
    }

    else if (filepack_is_tarball_backed(filepack)) {
        caValue* tarball = list_get(filepack, 1);
        tar_read_file(tarball, as_cstring(name), contents);
        return;
    }
    internal_error("filepack_read_file: filepack type not recognized");
}

bool filepack_does_file_exist(caValue* filepack, caValue* name)
{
    if (filepack_is_map(filepack)) {
        return hashtable_get(filepack, name) != NULL;
    }
    else if (filepack_is_filesystem_backed(filepack)) {
        Value fullPath;
        caValue* rootDir = list_get(filepack, 1);
        copy(rootDir, &fullPath);
        join_path(&fullPath, name);
        return file_exists(as_cstring(&fullPath));
    }
    else if (filepack_is_tarball_backed(filepack)) {
        caValue* tarball = list_get(filepack, 1);
        return tar_file_exists(tarball, as_cstring(name));
    }
    internal_error("filepack_does_file_exist: filepack type not recognized");
    return false;
}

int filepack_get_file_version(caValue* filepack, caValue* name)
{
    if (filepack_is_map(filepack)) {
        caValue* entry = hashtable_get(filepack, name);
        if (entry == NULL)
            return 0;

        return as_int(list_get(entry, 0));
    }
    else if (filepack_is_filesystem_backed(filepack)) {
        Value fullPath;
        caValue* rootDir = list_get(filepack, 1);
        copy(rootDir, &fullPath);
        join_path(&fullPath, name);
        return file_get_mtime(as_cstring(&fullPath));
    }
    else if (filepack_is_tarball_backed(filepack)) {
        // Currently no version support for tarballs.
        return 1;
    }
    return 0;
    internal_error("filepack_get_file_version: filepack type not recognized");
}

void filepack_create_using_filesystem(caValue* filepack, const char* rootDir)
{
    set_list(filepack, 2);
    set_symbol(list_get(filepack, 0), sym_Filesystem);
    set_string(list_get(filepack, 1), rootDir);
}

void filepack_create_from_tarball(caValue* filepack, caValue* blob)
{
    set_list(filepack, 2);
    set_symbol(list_get(filepack, 0), sym_Tarball);
    set_value(list_get(filepack, 1), blob);
}

CIRCA_EXPORT void circa_read_file(caWorld* world, const char* filename, caValue* contentsOut)
{
    Value name;
    set_string(&name, filename);

    caValue* fileSources = &world->fileSources;
    for (int i=list_length(fileSources) - 1; i >= 0; i--) {
        caValue* filepack = list_get(fileSources, i);
        filepack_read_file(filepack, &name, contentsOut);
        if (!is_null(contentsOut))
            return;
    }

    set_null(contentsOut);
}

CIRCA_EXPORT void circa_read_file_with_stack(caStack* stack, const char* filename, caValue* contentsOut)
{
    return ::circa_read_file(stack->world, filename, contentsOut);
}

CIRCA_EXPORT bool circa_file_exists(caWorld* world, const char* filename)
{
    Value name;
    set_string(&name, filename);

    caValue* fileSources = &world->fileSources;
    for (int i=list_length(fileSources) - 1; i >= 0; i--) {
        caValue* filepack = list_get(fileSources, i);
        bool exists = filepack_does_file_exist(filepack, &name);
        if (exists)
            return true;
    }

    return false;
}

CIRCA_EXPORT int circa_file_get_version(caWorld* world, const char* filename)
{
    Value name;
    set_string(&name, filename);

    caValue* fileSources = &world->fileSources;
    for (int i=list_length(fileSources) - 1; i >= 0; i--) {
        caValue* filepack = list_get(fileSources, i);
        if (!filepack_does_file_exist(filepack, &name))
            continue;

        return filepack_get_file_version(filepack, &name);
    }

    return 0;
}

} // namespace circa

