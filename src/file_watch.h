// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

/**
 * file_watch.h
 *
 * A FileWatch is an object that is responsible for 'watching' a single file. A watched
 * file is one that is actively being used by the runtime. Each FileWatch also stores
 * a list of actions, of things that should be done when the file is modified.
 *
 * The FileWatch system is a flexible way for the runtime to instantly react whenever a
 * relevant file is modified. This is used to instantly reload script files, native
 * modules, or other things.
 */

namespace circa {

struct FileWatchWorld;

FileWatchWorld* create_file_watch_world();

void start_watching_file(World* world, const char* filename);
void add_file_watch_action(World* world, const char* filename, Value* action);

} // namespace circa
