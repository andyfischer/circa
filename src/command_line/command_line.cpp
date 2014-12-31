// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "unistd.h"

#include "block.h"
#include "building.h"
#include "bytecode.h"
#include "debug.h"
#include "file.h"
#include "file_watch.h"
#include "inspection.h"
#include "interpreter.h"
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
        "  -b                  : Print out raw source with bytecode\n"
        "  -n                  : Don't actually run the script (for use with -p, -b, etc)\n"
        "  -print-state        : Print state as text after running the script\n"
        "  -break-on <id>      : Debugger break when term <id> is created\n"
        "\n"
        "Available commands:\n"
        "  -call <filename> <func name> <args>\n"
        "                    : Call a function in a script file, print results\n"
        "  -loop <filename>  : Run the script in an endless loop\n"
        "  -repl             : Start an interactive read-eval-print-loop\n"
        "  -check <filename> : Statically check the script for errors\n"
        "  -build <dir>      : Rebuild a module using a build.ca file\n"
        "  -run-stdin        : Read and execute commands from stdin\n"
        "  -source-repro     : Compile and reproduce a script's source (for testing)\n"
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
            rawOutputPrefs.showBytecode = true;
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

    if (string_equals(list_get(args, 0), "-call")) {
        Block* block = load_module_by_filename(world, list_get(args, 1));

        Stack* stack = circa_create_stack(world);

        // Push function
        caBlock* func = circa_find_function_local(block, as_cstring(list_get(args, 2)));
        circa_push_function(stack, func);

        // Push inputs
        for (int i=3, inputIndex = 0; i < circa_count(args); i++) {
            Value* val = circa_input(stack, inputIndex++);
            parse_string_repr(list_get(args, i), val);
        }

        circa_run(stack);

        if (circa_has_error(stack))
            circa_dump_stack_trace(stack);

        // Print outputs
        for (int i=0;; i++) {
            Value* out = circa_output(stack, i);
            if (out == NULL)
                break;

            dump(circa_output(stack, i));
        }
        
        circa_free_stack(stack);
        return 0;
    }

    if (string_equals(list_get(args, 0), "-loop")) {
        if (list_length(args) < 2) {
            std::cout << "Expected a filename after -loop" << std::endl;
            return 1;
        }

        Value* filename = list_get(args, 1);
        Block* block = load_module_by_filename(world, filename);
        Stack* stack = create_stack(world);
        stack_init(stack, block);
        while (true) {

            circa_run(stack);

            world_tick(world);

            // 16ms delay. TODO: Replace with libuv timer.
            usleep(16 * 1000);

            if (stack_errored(stack)) {
                std::cout << "Error occurred:\n";
                circa_dump_stack_trace(stack);
                std::cout << std::endl;
                std::cout << "Stack:\n";
                dump(stack);
                return 1;
            }
        }
        return 0;
    }

    // Run file checker
    if (string_equals(list_get(args, 0), "-check")) {
        Block* inputBlock = load_module_by_filename(world, list_get(args, 1));

        Value str_static_checking;
        set_string(&str_static_checking, "static_checking");
        Block* staticChecking = load_module(world, NULL, &str_static_checking);
        Block* func = find_function_local(staticChecking, "check_block_and_report");

        Stack stack;
        stack_init(&stack, func);
        set_block(circa_input(&stack, 0), inputBlock);

        circa_run(&stack);
        if (stack_errored(&stack))
            dump(&stack);
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

        Stack stack;
        stack_init(&stack, to_source_string);
        set_block(circa_input(&stack, 0), block);

        circa_run(&stack);

        if (stack_errored(&stack))
            dump(&stack);
        else
            printf("%s\n", as_cstring(circa_output(&stack, 0)));
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

    Stack* stack = create_stack(world);
    stack_init(stack, block);

    if (printRaw) {
        Value str;
        print_block(block, &rawOutputPrefs, &str, stack);
        printf("%s\n", as_cstring(&str));
    }

    if (dontRunScript)
        return 0;

    circa_run(stack);

    if (printState) {
        Value* state = stack_get_state(stack);
        if (state == NULL)
            std::cout << "state = null";
        else
            dump(state);
    }

    if (stack_errored(stack)) {
        std::cout << "Error occurred:\n";
        Value str;
        stack_trace_to_string(stack, &str);
        std::cout << as_cstring(&str);
        std::cout << std::endl;

        if (dumpOption) {
            std::cout << "Stack:\n";
            dump(stack);
        }

        return 1;
    }

    return 0;
}

void run_repl_stdin(World* world)
{
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

