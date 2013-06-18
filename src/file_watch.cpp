// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "debug.h"
#include "file.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "native_patch.h"
#include "tagged_value.h"
#include "world.h"

#define FILE_WATCH_DEBUG_LOGGING 0

namespace circa {

struct FileWatch
{
    Value filename;
    Value onChangeActions;
    int lastObservedMtime;
};

struct FileWatchWorld
{
    std::map<std::string, FileWatch*> watches;
};

FileWatchWorld* create_file_watch_world()
{
    FileWatchWorld* world = new FileWatchWorld();
    return world;
}

void dealloc_file_watch_world(FileWatchWorld* world)
{
    delete world;
}

FileWatch* find_file_watch(World* world, const char* filename)
{
    std::map<std::string, FileWatch*>::const_iterator it =
        world->fileWatchWorld->watches.find(filename);

    if (it != world->fileWatchWorld->watches.end())
        return it->second;

    return NULL;
}

FileWatch* add_file_watch(World* world, const char* filename)
{
    // Return existing watch if it exists.
    FileWatch* existing = find_file_watch(world, filename);
    if (existing != NULL)
        return existing;

#if FILE_WATCH_DEBUG_LOGGING
    printf("add_file_watch: %s\n", filename);
#endif

    FileWatch* newWatch = new FileWatch();
    set_string(&newWatch->filename, filename);
    set_list(&newWatch->onChangeActions, 0);
    newWatch->lastObservedMtime = 0;

    world->fileWatchWorld->watches[filename] = newWatch;
    return newWatch;
}

FileWatch* add_file_watch_action(World* world, const char* filename, Value* action)
{
    // Fetch the FileWatch entry.
    FileWatch* watch = add_file_watch(world, filename);

    // Check if this exact action already exists, if so do nothing.
    if (list_contains(&watch->onChangeActions, action))
        return watch;

    // Add action
    copy(action, list_append(&watch->onChangeActions));

    return watch;
}

static bool file_watch_check_for_update(FileWatch* watch)
{
    int latestMtime = file_get_mtime(as_cstring(&watch->filename));
    if (latestMtime != watch->lastObservedMtime) {
        watch->lastObservedMtime = latestMtime;
        return true;
    }

    return false;
}

void file_watch_trigger_actions(World* world, FileWatch* watch)
{
#if FILE_WATCH_DEBUG_LOGGING
    printf("file_watch_trigger_actions: %s\n", as_cstring(&watch->filename));
#endif

    // Walk through each action and execute it.
    for (int i = 0; i < list_length(&watch->onChangeActions); i++) {
        caValue* action = list_get(&watch->onChangeActions, i);

        Symbol label = first_symbol(action);
        ca_assert(label != sym_None);

        switch (label) {
        case sym_NativePatch: {
            caValue* moduleName = list_get(action, 1);

            NativePatch* nativeModule = add_native_patch(world, as_cstring(moduleName));
            native_patch_load_from_file(nativeModule, as_cstring(&watch->filename));
            native_patch_finish_change(nativeModule);
            break;
        }
        case sym_PatchBlock: {
            // Reload this code block.
            caValue* moduleName = list_get(action, 1);
            load_module_file(world, as_cstring(moduleName), as_cstring(&watch->filename));
            break;
        }
        default:
            internal_error("unrecognized file watch action");
        }
    }
}

void file_watch_trigger_actions(World* world, const char* filename)
{
    FileWatch* watch = find_file_watch(world, filename);

    // No-op if there is no watch.
    if (watch == NULL)
        return;

    file_watch_trigger_actions(world, watch);
}

void file_watch_check_now(World* world, FileWatch* watch)
{
    if (file_watch_check_for_update(watch))
        file_watch_trigger_actions(world, watch);
}

void file_watch_ignore_latest_change(FileWatch* watch)
{
    file_watch_check_for_update(watch);
}

void file_watch_check_all(World* world)
{
    std::map<std::string, FileWatch*>::const_iterator it;

    for (it = world->fileWatchWorld->watches.begin();
         it != world->fileWatchWorld->watches.end();
         ++it) {
        FileWatch* watch = it->second;
        file_watch_check_now(world, watch);
    }
}

FileWatch* add_file_watch_module_load(World* world, const char* filename, const char* moduleName)
{
    circa::Value action;
    set_list(&action, 2);
    set_symbol(list_get(&action, 0), sym_PatchBlock);
    set_string(list_get(&action, 1), moduleName);
    return add_file_watch_action(world, filename, &action);
}

FileWatch* add_file_watch_native_patch(World* world, const char* filename, const char* moduleName)
{
    circa::Value action;
    set_list(&action, 2);
    set_symbol(list_get(&action, 0), sym_NativePatch);
    set_string(list_get(&action, 1), moduleName);
    return add_file_watch_action(world, filename, &action);
}

CIRCA_EXPORT void circa_update_changed_files(caWorld* world)
{
    return file_watch_check_all(world);
}

} // namespace circa
