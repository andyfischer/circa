/**
 * file_watch.h
 *
 * The FileWatch system is a general purpose way to keep the runtime updated based on
 * changing files.
 *
 * For every file that the runtime actively cares about, there is a FileWatch object
 * (which is stored on the World). Every FileWatch has N different 'actions' which
 * are executed when the file is changed. Whenever the watched file is changed, all actions
 * are executed.
 *
 * For example, a watched Circa script file will have a reload-block action, which loads the
 * script and then updates the codebase to use the newly loaded code.
 *
 * Another file type that is supported out of the box, is the native module. When a watched
 * native module is changed, we'll reload the native functions and inject them into the
 * relevant code.
 *
 * Generally, a given FileWatch will only have 1 action. But if the same file is loaded for
 * different purposes then it might have more.
 *
 * A file watch can be created manually (such as with add_file_watch_action() and variants).
 * But usually the watch is created implicitly, such as when loading the module.
 *
 * The function file_watch_check_all() will look at every watched file, and runs the actions
 * for every file that has changed since the last call. This function should be called as often
 * as you want file changes to appear in the runtime.
 *
 * In the future we'll support efficient file change watching (such as with inotify), but for
 * now we simply load the file's modified-time on every check.
 *
 */

#pragma once

namespace circa {

struct FileWatchWorld;

FileWatchWorld* create_file_watch_world();
void dealloc_file_watch_world(FileWatchWorld* world);

// Add a file watch on the given file.
FileWatch* add_file_watch_action(World* world, const char* filename, Value* action);

// Immediately trigger the actions stored for the given file (if any).
void file_watch_trigger_actions(World* world, const char* filename);

// Check if the watched file has been updated, and run actions if so.
void file_watch_check_now(World* world, FileWatch* watch);

// Check if the watched file has been updated, and ignore this change if so.
void file_watch_ignore_latest_change(FileWatch* watch);

// Run all actions on recently modified files.
void file_watch_check_all(World* world);

// Add a watch to reload the given module.
FileWatch* add_file_watch_module_load(World* world, const char* filename, const char* moduleName);

// Add a watch to reload the given module.
FileWatch* add_file_watch_native_patch(World* world, const char* filename, const char* moduleName);

} // namespace circa
