// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#ifndef CIRCA_H_INCLUDED
#define CIRCA_H_INCLUDED

// Public API

#ifdef __cplusplus
extern "C" {
#endif

typedef int CircaSymbol;
typedef struct CircaTerm CircaTerm;

// Initialize Circa, should be called at startup before any other Circa functions.
void circa_initialize();

// Shutdown Circa, should be called when shutting down. This isn't required but it
// will make memory-leak checking tools happier.
void circa_shutdown();

// Tell runtime to use the standard filesystem interface when loading files.
// (without this call, the default behavior is to never touch the filesystem).
void circa_use_standard_filesystem();

// Add a module search path, used when processing 'import' statements.
void circa_add_module_search_path(const char* path);

// Execute with the given command-line args.
int circa_run_command_line(int argc, const char* args[]);

// Load a module by opening the given filename as a source file.
CircaTerm* circa_load_module_from_file(CircaSymbol module_name, const char* filename);

// Convert a string to a symbol value, creating it if necessary. Symbols are used
// internally as names.
CircaSymbol circa_string_to_symbol(const char* str);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
