// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

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
#include "source_repro.h"
#include "static_checking.h"
#include "string_repr.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "token.h"
#include "update_cascades.h"
#include "world.h"

#include "circa/file.h"
#include "debugger_repl.h"
#include "exporting_parser.h"
#include "file_checker.h"

#ifdef CIRCA_USE_LINENOISE
    extern "C" {
        #include "linenoise/linenoise.h"
    }
#endif

namespace circa {

void run_repl_stdin(World* world);

bool circa_get_line(caValue* lineOut)
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

void read_stdin_line(caValue* line)
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

void parse_string_as_argument_list(caValue* str, List* output)
{
    // Read the tokens as a space-seperated list of strings.
    // TODO is to be more smart about word boundaries: spaces inside
    // quotes or parentheses shouldn't break apart items.

    TokenStream tokens;
    tokens.reset(as_cstring(str));
    
    Value itemInProgress;
    set_string(&itemInProgress, "");

    while (!tokens.finished()) {

        if (tokens.nextIs(tok_Whitespace)) {
            if (!equals_string(&itemInProgress, "")) {
                copy(&itemInProgress, list_append(output));
                set_string(&itemInProgress, "");
            }

        } else {
            string_append(&itemInProgress, tokens.nextStr().c_str());
        }

        tokens.consume();
    }

    if (!equals_string(&itemInProgress, "")) {
        copy(&itemInProgress, list_append(output));
        set_string(&itemInProgress, "");
    }
}

void do_echo(List* args, caValue* reply)
{
    set_string(reply, to_string(args));
}

void do_file_command(caWorld* world, List* args, caValue* reply)
{
    RawOutputPrefs rawOutputPrefs;
    bool printRaw = false;
    bool printSource = false;
    bool printState = false;
    bool dontRunScript = false;

    int argIndex = 1;

    while (true) {

        if (argIndex >= args->length()) {
            set_string(reply, "No filename found");
            return;
        }

        if (string_eq(args->get(argIndex), "-p")) {
            printRaw = true;
            argIndex++;
            continue;
        }

        if (string_eq(args->get(argIndex), "-pp")) {
            printRaw = true;
            rawOutputPrefs.showProperties = true;
            argIndex++;
            continue;
        }
        
        if (string_eq(args->get(argIndex), "-b") || string_eq(args->get(argIndex), "-pb")) {
            printRaw = true;
            rawOutputPrefs.showBytecode = true;
            argIndex++;
            continue;
        }

        if (string_eq(args->get(argIndex), "-s")) {
            printSource = true;
            argIndex++;
            continue;
        }
        if (string_eq(args->get(argIndex), "-print-state")) {
            printState = true;
            argIndex++;
            continue;
        }
        if (string_eq(args->get(argIndex), "-n")) {
            dontRunScript = true;
            argIndex++;
            continue;
        }
        break;
    }

    Block block;
    load_script(&block, as_cstring(args->get(argIndex)));

    if (printSource)
        std::cout << get_block_source_text(&block);

    if (dontRunScript)
        return;
    
    Stack* stack = create_stack(world);
    evaluate_block(stack, &block);

    if (printState) {
        caValue* state = stack_get_state(stack);
        if (state == NULL)
            std::cout << "state = null";
        else
            std::cout << to_string(state) << std::endl;
    }

    if (stack_errored(stack)) {
        std::cout << "Error occurred:\n";
        Value str;
        stack_trace_to_string(stack, &str);
        std::cout << as_cstring(&str);
        std::cout << std::endl;
    }

    free_stack(stack);
}

void rewrite_block(Block* block, caValue* contents, caValue* reply)
{
    clear_block(block);
    parser::compile(block, parser::statement_list, as_cstring(contents));

    if (has_static_errors(block)) {
        std::stringstream errors;
        print_static_errors_formatted(block);
        set_string(reply, errors.str());
    } else {
        set_symbol(reply, sym_Success);
    }
}

void do_write_block(caValue* blockName, caValue* contents, caValue* reply)
{
    Term* term = find_global(as_cstring(blockName));

    // Create the block if needed
    if (term == NULL)
        term = apply(global_root_block(), FUNCS.section_block, TermList(), blockName);

    // Import the new block contents
    Block* block = nested_contents(term);
    rewrite_block(block, contents, reply);
}

void do_update_file(caValue* filename, caValue* contents, caValue* reply)
{
    Block* block = find_module_from_filename(as_cstring(filename));

    if (block == NULL) {
        set_string(reply, "Module not found");
        return;
    }

    rewrite_block(block, contents, reply);
}

void do_admin_command(caWorld* world, caValue* input, caValue* reply)
{
    // Identify the command
    int first_space = string_find_char(input, 0, ' ');
    if (first_space == -1)
        first_space = string_length(input);

    Value command;
    string_slice(input, 0, first_space, &command);

    set_null(reply);

    if (equals_string(&command, "add_lib_path")) {
        //List args;
        //parse_tokens_as_argument_list(&tokens, &args);

    } else if (equals_string(&command, "file")) {

        List args;
        parse_string_as_argument_list(input, &args);
        do_file_command(world, &args, reply);

    } else if (equals_string(&command, "echo")) {

        List args;
        parse_string_as_argument_list(input, &args);
        do_echo(&args, reply);

    } else if (equals_string(&command, "write_block")) {

        int nextSpace = string_find_char(input, first_space+1, ' ');
        if (nextSpace == -1) {
            set_string(reply, "Syntax error, not enough arguments");
            return;
        }
        
        Value blockName;
        string_slice(input, first_space+1, nextSpace, &blockName);

        Value contents;
        string_slice(input, nextSpace+1, -1, &contents);

        do_write_block(&blockName, &contents, reply);

    } else if (equals_string(&command, "update_file")) {

        int nextSpace = string_find_char(input, first_space+1, ' ');
        if (nextSpace == -1) {
            set_string(reply, "Syntax error, not enough arguments");
            return;
        }
        
        Value filename;
        string_slice(input, first_space+1, nextSpace, &filename);

        Value contents;
        string_slice(input, nextSpace+1, -1, &contents);

        do_update_file(&filename, &contents, reply);

    } else if (equals_string(&command, "source_repro")) {
        List args;
        parse_string_as_argument_list(input, &args);
        Block block;
        load_script(&block, as_cstring(args[1]));
        std::cout << get_block_source_text(&block);
    } else if (equals_string(&command, "dump_stats")) {

        perf_stats_dump();
        std::cout << ":done" << std::endl;

    } else {

        set_string(reply, "Unrecognized command: ");
        string_append(reply, &command);
    }
}

void run_commands_from_stdin(caWorld* world)
{
    while (true) {
        Value line;
        read_stdin_line(&line);
        if (!is_string(&line))
            break;

        Value reply;
        do_admin_command(world, &line, &reply);

        if (is_null(&reply))
            ; // no op
        else if (is_string(&reply))
            std::cout << as_string(&reply) << std::endl;
        else
            std::cout << to_string(&reply) << std::endl;

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
        "  -repl             : Start an interactive read-eval-print-loop\n"
        "  -check <filename> : Statically check the script for errors\n"
        "  -build <dir>      : Rebuild a module using a build.ca file\n"
        "  -run-stdin        : Read and execute commands from stdin\n"
        "  -source-repro     : Compile and reproduce a script's source (for testing)\n"
        << std::endl;
}

int run_command_line(caWorld* world, caValue* args)
{
    RawOutputPrefs rawOutputPrefs;
    bool printRaw = false;
    bool printState = false;
    bool dontRunScript = false;
    bool printTrace = false;

    // Prepended options
    while (true) {

        if (list_length(args) == 0)
            break;

        if (string_eq(list_get(args, 0), "-break-on")) {
            DEBUG_BREAK_ON_TERM = atoi(as_cstring(list_get(args, 1)));

            list_remove_index(args, 0);
            list_remove_index(args, 0);
            std::cout << "breaking on creation of term: " << DEBUG_BREAK_ON_TERM << std::endl;
            continue;
        }

        if (string_eq(list_get(args, 0), "-path")) {
            // Add a module path
            module_add_search_path(world, as_cstring(list_get(args, 1)));
            list_remove_index(args, 0);
            list_remove_index(args, 0);
            continue;
        }

        if (string_eq(list_get(args, 0), "-p")) {
            printRaw = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_eq(list_get(args, 0), "-pp")) {
            printRaw = true;
            rawOutputPrefs.showProperties = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_eq(list_get(args, 0), "-b") || string_eq(list_get(args, 0), "-pb")) {
            printRaw = true;
            rawOutputPrefs.showBytecode = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_eq(list_get(args, 0), "-n")) {
            dontRunScript = true;
            list_remove_index(args, 0);
            continue;
        }
        if (string_eq(list_get(args, 0), "-print-state")) {
            printState = true;
            list_remove_index(args, 0);
            continue;
        }
        if (string_eq(list_get(args, 0), "-t")) {
            printTrace = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_eq(list_get(args, 0), "-load")) {
            caValue* filename = list_get(args, 1);

            Value moduleName;
            module_get_default_name_from_filename(filename, &moduleName);

            list_remove_index(args, 0);
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
    
    Block* mainBlock = fetch_module(world, "main");

    // Check to handle args[0] as a dash-command.

    // Print help
    if (string_eq(list_get(args, 0), "-help")) {
        print_usage();
        return 0;
    }

    // Start repl
    if (string_eq(list_get(args, 0), "-repl")) {
        run_repl_stdin(world);
        return 0;
    }

    if (string_eq(list_get(args, 0), "-call")) {
        Symbol loadResult = load_script(mainBlock, as_cstring(list_get(args, 1)));

        if (loadResult == sym_Failure) {
            std::cout << "Failed to load file: " <<  as_cstring(list_get(args, 1)) << std::endl;
            return -1;
        }

        block_finish_changes(mainBlock);

        caStack* stack = circa_create_stack(world);

        // Push function
        caBlock* func = circa_find_function_local(mainBlock, as_cstring(list_get(args, 2)));
        circa_push_function(stack, func);

        // Push inputs
        for (int i=3, inputIndex = 0; i < circa_count(args); i++) {
            caValue* val = circa_input(stack, inputIndex++);
            parse_string_repr(as_cstring(list_get(args, i)), val);
        }

        circa_run(stack);

        if (circa_has_error(stack))
            circa_dump_stack_trace(stack);

        // Print outputs
        for (int i=0;; i++) {
            caValue* out = circa_output(stack, i);
            if (out == NULL)
                break;

            std::cout << to_string(circa_output(stack, i)) << std::endl;
        }
        
        circa_free_stack(stack);
    }

    // Start debugger repl
    if (string_eq(list_get(args, 0), "-d"))
        return run_debugger_repl(as_cstring(list_get(args, 1)));

    // Run file checker
    if (string_eq(list_get(args, 0), "-check"))
        return run_file_checker(as_cstring(list_get(args, 1)));

    // Export parsed information
    if (string_eq(list_get(args, 0), "-export")) {
        const char* filename = "";
        const char* format = "";
        if (list_length(args) >= 2)
            format = as_cstring(list_get(args, 1));
        if (list_length(args) >= 3)
            filename = as_cstring(list_get(args, 2));
        return run_exporting_parser(format, filename);
    }

    // Command reader (from stdin)
    if (string_eq(list_get(args, 0), "-run-stdin")) {
        run_commands_from_stdin(world);
        return 0;
    }

    // Reproduce source text
    if (string_eq(list_get(args, 0), "-source-repro")) {
        load_script(mainBlock, as_cstring(list_get(args, 1)));
        std::cout << get_block_source_text(mainBlock);
        return 0;
    }

    // Rewrite source, this is useful for upgrading old source
    if (string_eq(list_get(args, 0), "-rewrite-source")) {
        load_script(mainBlock, as_cstring(list_get(args, 1)));
        std::string contents = get_block_source_text(mainBlock);
        write_text_file(as_cstring(list_get(args, 1)), contents.c_str());
        return 0;
    }

    // Default behavior with no flags: run args[0] as a script filename.

    caValue* filename = list_get(args, 0);

    load_script(mainBlock, as_cstring(filename));
    block_finish_changes(mainBlock);
    refresh_bytecode(mainBlock);

    if (printRaw)
        print_block(mainBlock, &rawOutputPrefs, std::cout);

    if (dontRunScript)
        return 0;

    Stack* stack = create_stack(world);
    stack_init(stack, mainBlock);

    run_interpreter(stack);

    if (printState) {
        caValue* state = stack_get_state(stack);
        if (state == NULL)
            std::cout << "state = null";
        else
            std::cout << to_string(state) << std::endl;
    }

    if (stack_errored(stack)) {
        std::cout << "Error occurred:\n";
        circa_dump_stack_trace(stack);
        std::cout << std::endl;
        std::cout << "Stack:\n";
        dump(stack);
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
        if (stack_top(stack) == NULL)
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

