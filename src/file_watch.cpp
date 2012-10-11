// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

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
}

FileWatch* add_file_watch(World* world, const char* filename)
{
    std::map<std::string, FileWatch*>::const_iterator it =
        world->fileWatchWorld->watches.find(filename);

    // Return existing watch if it exists.
    if (it != world->fileWatchWorld->watches.end())
        return it->second;

    FileWatch* newWatch = new FileWatch();
    set_string(&newWatch->filename, filename);
    set_list(&newWatch->onChangeActions, 0);
    newWatch->last_known_mtime = 0;

    world->fileWatchWorld->watches[filename] = newWatch;
    return newWatch;
}

void add_file_watch_action(World* world, const char* filename, Value* action)
{
}

} // namespace circa
