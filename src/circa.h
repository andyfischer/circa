// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "code_iterators.h"
#include "codegen.h"
#include "debug.h"
#include "dict.h"
#include "documentation.h"
#include "evaluation.h"
#include "feedback.h"
#include "filesystem.h"
#include "for_loop.h"
#include "function.h"
#include "generic.h"
#include "if_block.h"
#include "kernel.h"
#include "importing.h"
#include "importing_macros.h"
#include "introspection.h"
#include "list_shared.h"
#include "locals.h"
#include "names.h"
#include "parser.h"
#include "refactoring.h"
#include "stateful_code.h"
#include "static_checking.h"
#include "source_repro.h"
#include "subroutine.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "term_source_location.h"
#include "testing.h"
#include "token.h"
#include "token_stream.h"
#include "type.h"
#include "type_inference.h"

#include "types/handle.h"
#include "types/list.h"
#include "types/point.h"
#include "types/rect.h"
#include "types/ref.h"

// C-style public API
extern "C" {

// Initialize Circa, should be called at startup, before any other Circa functions.
void circa_initialize();

// Shutdown Circa, should be called when shutting down. This isn't required but it
// will make memory-leak checking tools happier.
void circa_shutdown();

// Tell runtime to use the standard filesystem interface when loading files. (by
// default, the runtime won't touch the filesystem).
void circa_use_standard_filesystem();

// Add a module search path, used when processing 'import' statements.
void circa_add_module_search_path(const char* path);

// Execute with the given command-line args.
int circa_run_command_line(int argc, const char* args[]);

circa::Term* circa_load_module_from_file(circa::Symbol module_name, const char* filename);
circa::Symbol circa_string_to_symbol(const char* str);

}
