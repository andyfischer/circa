// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "list.h"
#include "tagged_value.h"
#include "world.h"

namespace circa {

struct FileWatch
{
    Value filename;
    Value onChangeActions;
    int last_known_mtime;
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

    FileWatch* newWatch = new FileWatch();
    set_string(&newWatch->filename, filename);
    set_list(&newWatch->onChangeActions, 0);
    newWatch->last_known_mtime = 0;

    world->fileWatchWorld->watches[filename] = newWatch;
    return newWatch;
}

void add_file_watch_action(World* world, const char* filename, Value* action)
{
    // Fetch the FileWatch entry.
    FileWatch* watch = add_file_watch(world, filename);

    // Check if this exact action already exists, if so do nothing.
    for (int i=0; i < list_length(&watch->onChangeActions); i++)
        if (equals(list_get(&watch->onChangeActions, i), action))
            return;

    // Add action
    copy(action, list_append(&watch->onChangeActions));
}

void file_watch_trigger_actions(World* world, const char* filename)
{
    FileWatch* watch = find_file_watch(world, filename);

    // No-op if there is no watch.
    if (watch == NULL)
        return;

    // TODO - walk through each action.
}

void file_watch_check_all(World* world)
{
}

} // namespace circa
