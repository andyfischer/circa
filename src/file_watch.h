// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

/**
 * file_watch.h
 *
 * A 'file watch' is an object that is responsible for 'watching' a single file.
 * Each object has a list of actions. When the file is modified, we run the stored
 * actions.
 */

namespace circa {

struct FileWatchWorld;

FileWatchWorld* create_file_watch_world();

void start_watching_file(World* world, const char* filename);
void add_file_watch_action(World* world, const char* filename, Value* action);

} // namespace circa
