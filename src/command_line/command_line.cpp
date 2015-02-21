// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "unistd.h"

#include "block.h"
#include "building.h"
#include "bytecode2.h"
#include "debug.h"
#include "file.h"
#include "file_watch.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "parser.h"
#include "repl.h"
#include "static_checking.h"
#include "string_repr.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "token.h"
#include "update_cascades.h"
#include "vm.h"
#include "world.h"

#include "circa/file.h"
#include "file_checker.h"

#include "command_line.h"

#ifdef CIRCA_USE_LINENOISE
    extern "C" {
        #include "linenoise/linenoise.h"
    }
#endif

namespace circa {

void run_repl_stdin(World* world);

bool circa_get_line(Value* lineOut)
{
#ifdef CIRCA_USE_LINENOISE
    char* input = linenoise("> ");

    if (input == NULL)
        return false;

    linenoiseHistoryAdd(input);

    set_string(lineOut, input);
    free(input);
    return true;

#else
    internal_error("This tool was built without line reader support");
#endif

    return true;
}

void read_stdin_line(Value* line)
{
    char* buf = NULL;
    size_t size = 0;
    ssize_t read = getline(&buf, &size, stdin);

    if (read == -1) {
        set_null(line);
        free(buf);
        return;
    }

    buf[read] = 0;

    // Truncate newline
    if (read > 0 && buf[read-1] == '\n')
        buf[read - 1] = 0;

    set_string(line, buf);
    free(buf);
}

void parse_string_as_argument_list(Value* str, Value* output)
{
    // Read the tokens as a space-seperated list of strings.
    // TODO is to be more smart about word boundaries: spaces inside
    // quotes or parentheses shouldn't break apart items.

    if (!is_list(output))
        set_list(output, 0);

    TokenStream tokens(str);
    
    Value itemInProgress;
    set_string(&itemInProgress, "");

    while (!tokens.finished()) {

        if (tokens.nextIs(tok_Whitespace)) {
            if (!equals_string(&itemInProgress, "")) {
                copy(&itemInProgress, list_append(output));
                set_string(&itemInProgress, "");
            }

        } else {
            Value next;
            tokens.getNextStr(&next);
            string_append(&itemInProgress, &next);
        }

        tokens.consume();
    }

    if (!equals_string(&itemInProgress, "")) {
        copy(&itemInProgress, list_append(output));
        set_string(&itemInProgress, "");
    }
}

void run_commands_from_stdin(caWorld* world)
{
    while (true) {
        Value line;
        read_stdin_line(&line);
        if (!is_string(&line))
            break;

        Value args;
        parse_string_as_argument_list(&line, &args);
        run_command_line(world, &args);

        // We need to tell the stdout reader that we have finished. The proper
        // way to do this would probably be to format the entire output as an
        // escapted string. But, this is what we'll do for now:
        std::cout << ":done" << std::endl;
    }
}

void print_usage()
{
    std::cout <<
        "Usage:\n"
        "  circa <options> <filename>\n"
        "  circa <options> <dash-command> <command args>\n"
        "\n"
        "Available options:\n"
        "  -path <path>     : Add a module search path\n"
        "  -p                  : Print out raw source\n"
        "  -pp                 : Print out raw source with properties\n"
        "  -n                  : Don't actually run the script (for use with -p, etc)\n"
        "  -print-state        : Print state as text after running the script\n"
        "  -break-on <id>      : Debugger break when term <id> is created\n"
        "\n"
        "Available commands:\n"
        "  -repl             : Start an interactive read-eval-print-loop\n"
        "  -check <filename> : Statically check the script for errors\n"
        "  -run-stdin        : Read and execute commands from stdin\n"
        << std::endl;
}

int run_command_line(caWorld* world, Value* args)
{
    RawOutputPrefs rawOutputPrefs;
    bool printRaw = false;
    bool printState = false;
    bool dontRunScript = false;
    bool printTrace = false;
    bool dumpOption = false;

    // Prepended options
    while (true) {

        if (list_length(args) == 0)
            break;

        if (string_equals(list_get(args, 0), "-break-on")) {
            DEBUG_BREAK_ON_TERM = atoi(as_cstring(list_get(args, 1)));

            list_remove_index(args, 0);
            list_remove_index(args, 0);
            std::cout << "breaking on creation of term: " << DEBUG_BREAK_ON_TERM << std::endl;
            continue;
        }

        if (string_equals(list_get(args, 0), "-path")) {
            // Add a module path
            module_add_search_path(world, as_cstring(list_get(args, 1)));
            list_remove_index(args, 0);
            list_remove_index(args, 0);
            continue;
        }

        if (string_equals(list_get(args, 0), "-p")) {
            printRaw = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_equals(list_get(args, 0), "-pp")) {
            printRaw = true;
            rawOutputPrefs.showProperties = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_equals(list_get(args, 0), "-b") || string_equals(list_get(args, 0), "-pb")) {
            printRaw = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_equals(list_get(args, 0), "-n")) {
            dontRunScript = true;
            list_remove_index(args, 0);
            continue;
        }
        if (string_equals(list_get(args, 0), "-print-state")) {
            printState = true;
            list_remove_index(args, 0);
            continue;
        }
        if (string_equals(list_get(args, 0), "-t")) {
            printTrace = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_equals(list_get(args, 0), "-load")) {
            Value* filename = list_get(args, 1);
            load_module(world, NULL, filename);

            list_remove_index(args, 0);
            list_remove_index(args, 0);
            continue;
        }

        if (string_equals(list_get(args, 0), "-dump")) {
            dumpOption = true;
            list_remove_index(args, 0);
            continue;
        }

        break;
    }

    // No arguments remaining
    if (list_length(args) == 0) {
        print_usage();
        return 0;
    }
    
    // Check to handle args[0] as a dash-command.

    // Print help
    if (string_equals(list_get(args, 0), "-help")) {
        print_usage();
        return 0;
    }

    // Start repl
    if (string_equals(list_get(args, 0), "-repl")) {
        run_repl_stdin(world);
        return 0;
    }

    // Command reader (from stdin)
    if (string_equals(list_get(args, 0), "-run-stdin")) {
        run_commands_from_stdin(world);
        return 0;
    }

    // Reproduce source text
    if (string_equals(list_get(args, 0), "-source-repro")) {
        Block* block = load_module_by_filename(world, list_get(args, 1));

        Value sourceReproStr;
        set_string(&sourceReproStr, "source_repro");
        Block* sourceRepro = load_module(world, NULL, &sourceReproStr);
        Block* to_source_string = find_function_local(sourceRepro, "block_to_string");

        VM* vm = new_vm(to_source_string);
        set_block(vm->input(0), block);

        vm_run(vm, NULL);

        if (vm_has_error(vm))
            vm->dump();
        else
            printf("%s\n", as_cstring(vm->output()));

        free_vm(vm);
        
        return 0;
    }

    // Default behavior with no flags: run args[0] as a script filename.

    Value* arg = list_get(args, 0);
    Value filename;
    resolve_possible_module_path(world, arg, &filename);

    if (is_null(&filename)) {
        printf("Local module not found: %s\n", as_cstring(arg));
        return -1;
    }

    Block* block = load_module_by_filename(world, &filename);

    VM* vm = new_vm(block);

    if (printRaw) {
        Value str;
        print_block(block, &rawOutputPrefs, &str);
        printf("%s\n", as_cstring(&str));
    }

    if (dontRunScript)
        return 0;

    vm_run(vm, NULL);

    if (vm->has_error()) {
        printf("Error occurred: %s\n", vm_get_error(vm)->to_c_string());
        return 1;
    }

    return 0;
}

void run_repl_stdin(World* world)
{
    printf("REPL no worky\n");
#if 0
    Stack* stack = create_stack(world);
    repl_start(stack);

    printf("Started REPL, type /help for reference.\n");

    while (true) {
        Value input;

        // Get next line
        if (!circa_get_line(&input))
            break;

        // Before doing any work, process any pending file changes.
        file_watch_check_all(world);

        Value output;
        repl_run_line(stack, &input, &output);

        for (int i=0; i < list_length(&output); i++)
            std::cout << as_cstring(list_get(&output, i)) << std::endl;

        // Check if we've finished.
        if (top_frame(stack) == NULL)
            return;
    }
#endif
}

int run_command_line(caWorld* world, int argc, const char* args[])
{
    Value args_v;
    set_list(&args_v, 0);
    for (int i=1; i < argc; i++)
        circa_set_string(circa_append(&args_v), args[i]);

    return run_command_line(world, &args_v);
}

} // namespace circa

