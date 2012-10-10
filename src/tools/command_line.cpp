// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "debug.h"
#include "evaluation.h"
#include "file_utils.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "parser.h"
#include "source_repro.h"
#include "static_checking.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "token.h"
#include "update_cascades.h"

#include "circa/file.h"
#include "build_tool.h"
#include "codegen.h"
#include "debugger_repl.h"
#include "exporting_parser.h"
#include "generate_cpp.h"
#include "file_checker.h"
#include "repl.h"

namespace circa {

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

void do_add_lib_path(List* args, caValue* reply)
{
    modules_add_search_path(as_cstring(list_get(args, 0)));
}

void do_echo(List* args, caValue* reply)
{
    set_string(reply, to_string(args));
}

void do_file_command(List* args, caValue* reply)
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

    Branch branch;
    load_script(&branch, as_cstring(args->get(argIndex)));

    if (printSource)
        std::cout << get_branch_source_text(&branch);

    if (dontRunScript)
        return;
    
    Stack stack;
    evaluate_branch(&stack, &branch);

    if (printState)
        std::cout << to_string(&stack.state) << std::endl;

    if (error_occurred(&stack)) {
        std::cout << "Error occurred:\n";
        print_error_stack(&stack, std::cout);
        std::cout << std::endl;
        return;
    }
}

void rewrite_branch(Branch* branch, caValue* contents, caValue* reply)
{
    clear_branch(branch);
    parser::compile(branch, parser::statement_list, as_cstring(contents));

    if (has_static_errors(branch)) {
        std::stringstream errors;
        print_static_errors_formatted(branch);
        set_string(reply, errors.str());
    } else {
        set_name(reply, name_Success);
    }
}

void do_write_branch(caValue* branchName, caValue* contents, caValue* reply)
{
    Name name = name_from_string(branchName);

    Term* term = get_global(name);

    // Create the branch if needed
    if (term == NULL) {
        term = apply(kernel(), FUNCS.branch, TermList(), name);
    }

    // Import the new branch contents
    Branch* branch = nested_contents(term);
    rewrite_branch(branch, contents, reply);
}

void do_update_file(caValue* filename, caValue* contents, caValue* reply)
{
    Branch* branch = find_module_from_filename(as_cstring(filename));

    if (branch == NULL) {
        set_string(reply, "Module not found");
        return;
    }

    rewrite_branch(branch, contents, reply);
}

void do_admin_command(caValue* input, caValue* reply)
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
        do_file_command(&args, reply);

    } else if (equals_string(&command, "echo")) {

        List args;
        parse_string_as_argument_list(input, &args);
        do_echo(&args, reply);

    } else if (equals_string(&command, "write_branch")) {

        int nextSpace = string_find_char(input, first_space+1, ' ');
        if (nextSpace == -1) {
            set_string(reply, "Syntax error, not enough arguments");
            return;
        }
        
        Value branchName;
        string_slice(input, first_space+1, nextSpace, &branchName);

        Value contents;
        string_slice(input, nextSpace+1, -1, &contents);

        do_write_branch(&branchName, &contents, reply);

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
        Branch branch;
        load_script(&branch, as_cstring(args[1]));
        std::cout << get_branch_source_text(&branch);
    } else if (equals_string(&command, "dump_stats")) {

        perf_stats_dump();
        std::cout << ":done" << std::endl;

    } else {

        set_string(reply, "Unrecognized command: ");
        string_append(reply, &command);
    }
}

void run_commands_from_stdin()
{
    while (true) {
        Value line;
        read_stdin_line(&line);
        if (!is_string(&line))
            break;

        Value reply;
        do_admin_command(&line, &reply);

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
        "  -libpath <path>     : Add a module search path\n"
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
        "  -e <expression>   : Evaluate an expression on the command line\n"
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

        if (string_eq(list_get(args, 0), "-libpath")) {
            // Add a module path
            modules_add_search_path(as_cstring(list_get(args, 1)));
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

        break;
    }

    // No arguments remaining
    if (list_length(args) == 0) {
#ifdef CIRCA_TEST_BUILD
        run_all_tests();
#else
        print_usage();
#endif
        return 0;
    }

    // Check to handle args[0] as a dash-command.

    // Print help
    if (string_eq(list_get(args, 0), "-help")) {
        print_usage();
        return 0;
    }

    // Eval mode
    if (string_eq(list_get(args, 0), "-e")) {
        list_remove_index(args, 0);

        Value command;
        set_string(&command, "");

        bool firstArg = true;
        while (!list_empty(args)) {
            if (!firstArg)
                string_append(&command, " ");
            string_append(&command, list_get(args, 0));
            list_remove_index(args, 0);
            firstArg = false;
        }

        Branch workspace;
        caValue* result = term_value(workspace.eval(as_cstring(&command)));
        std::cout << to_string(result) << std::endl;
        return 0;
    }

    // Start repl
    if (string_eq(list_get(args, 0), "-repl"))
        return run_repl();

    if (string_eq(list_get(args, 0), "-call")) {
        Branch* branch = create_branch(kernel());
        Name loadResult = load_script(branch, as_cstring(list_get(args, 1)));

        if (loadResult == name_Failure) {
            std::cout << "Failed to load file: " <<  as_cstring(list_get(args, 1)) << std::endl;
            return -1;
        }

        branch_finish_changes(branch);

        caStack* stack = circa_alloc_stack(world);

        // Push function
        caFunction* func = circa_find_function(branch, as_cstring(list_get(args, 2)));
        circa_push_function(stack, func);

        // Push inputs
        for (int i=3, inputIndex = 0; i < circa_count(args); i++) {
            caValue* val = circa_input(stack, inputIndex++);
            circa_parse_string(as_cstring(list_get(args, i)), val);
        }

        circa_run(stack);

        if (circa_has_error(stack)) {
            circa_print_error_to_stdout(stack);
        }

        // Print outputs
        for (int i=0;; i++) {
            caValue* out = circa_output(stack, i);
            if (out == NULL)
                break;

            std::cout << to_string(circa_output(stack, i)) << std::endl;
        }
        
        circa_dealloc_stack(stack);
    }

    // Start debugger repl
    if (string_eq(list_get(args, 0), "-d"))
        return run_debugger_repl(as_cstring(list_get(args, 1)));

    // Generate cpp headers
    if (string_eq(list_get(args, 0), "-gh")) {
        Branch branch;
        load_script(&branch, as_cstring(list_get(args, 1)));

        std::cout << generate_cpp_headers(&branch);

        return 0;
    }

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

    // Build tool
    if (string_eq(list_get(args, 0), "-build")) {
        return run_build_tool(args);
    }

    // C++ gen
    if (string_eq(list_get(args, 0), "-cppgen")) {
        Value remainingArgs;
        list_slice(args, 1, -1, &remainingArgs);
        run_generate_cpp(&remainingArgs);
        return 0;
    }

    // Command reader (from stdin)
    if (string_eq(list_get(args, 0), "-run-stdin")) {
        run_commands_from_stdin();
        return 0;
    }

    // Reproduce source text
    if (string_eq(list_get(args, 0), "-source-repro")) {
        Branch branch;
        load_script(&branch, as_cstring(list_get(args, 1)));
        std::cout << get_branch_source_text(&branch);
        return 0;
    }

    // Rewrite source, this is useful for upgrading old source
    if (string_eq(list_get(args, 0), "-rewrite-source")) {
        Branch branch;
        load_script(&branch, as_cstring(list_get(args, 1)));
        std::string contents = get_branch_source_text(&branch);
        circa_write_text_file(as_cstring(list_get(args, 1)), contents.c_str());
        return 0;
    }

    // Default behavior with no flags: load args[0] as a script and run it.
    Branch* main_branch = create_branch(kernel());
    load_script(main_branch, as_cstring(list_get(args, 0)));
    branch_finish_changes(main_branch);
    refresh_bytecode(main_branch);

    if (printRaw) {
        print_branch(std::cout, main_branch, &rawOutputPrefs);
    }

    if (dontRunScript)
        return 0;

    Stack* stack = alloc_stack(world);

    push_frame(stack, main_branch);

    run_interpreter(stack);

    if (printState)
        std::cout << to_string(&stack->state) << std::endl;

    if (error_occurred(stack)) {
        std::cout << "Error occurred:\n";
        print_error_stack(stack, std::cout);
        std::cout << std::endl;
        std::cout << "Stack:\n";
        dump(stack);
        return 1;
    }

    return 0;
}

EXPORT int circa_run_command_line(caWorld* world, int argc, const char* args[])
{
    circa::Value args_v;
    circa_set_list(&args_v, 0);
    for (int i=1; i < argc; i++)
        circa_set_string(circa_append(&args_v), args[i]);

    return circa::run_command_line(world, &args_v);
}

} // namespace circa

int main(int argc, const char * args[])
{
    caWorld* world = circa_initialize();

    int result = 0;
    result = circa_run_command_line(world, argc, args);

    circa_shutdown(world);

    // temp
    // circa::perf_stats_dump();
    
    return result;
}
