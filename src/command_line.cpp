// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"
#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "codegen.h"
#include "evaluation.h"
#include "feedback.h"
#include "introspection.h"
#include "list_shared.h"
#include "modules.h"
#include "parser.h"
#include "source_repro.h"
#include "static_checking.h"
#include "string_type.h"
#include "testing.h"

#include "tools/build_tool.h"
#include "tools/command_reader.h"
#include "tools/generate_cpp.h"
#include "tools/debugger_repl.h"
#include "tools/exporting_parser.h"
#include "tools/file_checker.h"
#include "tools/repl.h"

namespace circa {

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
        "  -s                  : Print out reconstructed source code (for testing)\n"
        "  -n                  : Don't actually run the script (for use with -p, -pp or -s)\n"
        "  -break-on <id>      : Debugger break when term <id> is created\n"
        "  -print-state        : Print state as text after running the script\n"
        "\n"
        "Available commands:\n"
        "  -loop <filename>  : Run the script repeatedly until terminated\n"
        "  -repl             : Start an interactive read-eval-print-loop\n"
        "  -e <expression>   : Evaluate an expression on the command line\n"
        "  -check <filename> : Statically check the script for any errors\n"
        "  -build <dir>      : Rebuild a module using a build.ca file\n"
        "  -run-stdin        : Read and execute commands from stdin\n"
#ifdef CIRCA_TEST_BUILD
        "  -test             : Run unit tests\n"
        "  -test <name>      : Run unit test of a certain name\n"
        "  -list-tests       : List every unit test name\n"
#endif
        << std::endl;
}

int run_command_line(List* args)
{
    bool printRaw = false;
    bool printRawWithProps = false;
    bool printSource = false;
    bool printState = false;
    bool dontRunScript = false;

    // Prepended options
    while (true) {

        if (list_length(args) == 0)
            break;

        if (string_eq(args->get(0), "-break-on")) {
            String name;
            set_string(&name, "$");
            string_append(&name, (String*) args->get(1));
            DEBUG_BREAK_ON_TERM = strdup(as_cstring(&name));

            list_remove_index(args, 0);
            list_remove_index(args, 0);
            std::cout << "breaking on creation of term: " << DEBUG_BREAK_ON_TERM << std::endl;
            continue;
        }

        if (string_eq(args->get(0), "-libpath")) {
            // Add a module path
            modules_add_search_path(as_cstring(args->get(1)));
            list_remove_index(args, 0);
            list_remove_index(args, 0);
            continue;
        }

        if (string_eq(args->get(0), "-p")) {
            printRaw = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_eq(args->get(0), "-pp")) {
            printRawWithProps = true;
            list_remove_index(args, 0);
            continue;
        }

        if (string_eq(args->get(0), "-s")) {
            printSource = true;
            list_remove_index(args, 0);
            continue;
        }
        if (string_eq(args->get(0), "-n")) {
            dontRunScript = true;
            list_remove_index(args, 0);
            continue;
        }
        if (string_eq(args->get(0), "-print-state")) {
            printState = true;
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

#ifdef CIRCA_TEST_BUILD
    // Run unit tests
    if (string_eq(args->get(0), "-test")) {
        if (args->length() > 1)
            run_tests(as_cstring(args->get(1)));
        else
            run_all_tests();
        return 0;
    }
#endif

    // Print help
    if (string_eq(args->get(0), "-help")) {
        print_usage();
        return 0;
    }

    // Eval mode
    if (string_eq(args->get(0), "-e")) {
        list_remove_index(args, 0);

        String command;

        bool firstArg = true;
        while (!args->empty()) {
            if (!firstArg)
                string_append(&command, " ");
            string_append(&command, args->get(0));
            list_remove_index(args, 0);
            firstArg = false;
        }

        Branch workspace;
        TValue* result = workspace.eval(as_cstring(&command));
        std::cout << result->toString() << std::endl;
        return 0;
    }

#ifdef CIRCA_TEST_BUILD
    if (string_eq(args->get(0), "-list-tests")) {
        std::vector<std::string> testNames = list_all_test_names();

        std::vector<std::string>::const_iterator it;
        for (it = testNames.begin(); it != testNames.end(); ++it) {
            std::cout << *it << std::endl;
        }
        return 0;
    }
#endif

    // Start repl
    if (string_eq(args->get(0), "-repl"))
        return run_repl();

    // Start evaluation loop
    if (string_eq(args->get(0), "-loop")) {
        Branch branch;
        load_script(&branch, as_cstring(list_get_index(args, 1)));

        if (has_static_errors(&branch)) {
            print_static_errors_formatted(&branch, std::cout);
            return -1;
        }

        EvalContext context;

        while (true) {
            if (error_occurred(&context)) {
                print_runtime_error_formatted(&context, std::cout);
                return -1;
            }

            evaluate_branch(&context, &branch);

            sleep(1);
        }
    }

    // Start debugger repl
    if (string_eq(args->get(0), "-d"))
        return run_debugger_repl(as_cstring(args->get(1)));

    // Do a feedback test (disabled)
    #if 0
    if (args[0] == "-f") {
        Branch branch;
        load_script(&branch, args[1]);

        Branch &trainable_names = branch["_trainable"]->nestedContents;
        for (int i=0; i < trainable_names.length(); i++)
            set_trainable(branch[trainable_names[i]->asString()], true);
        refresh_training_branch(branch);

        std::cout << std::endl;
        std::cout << "-- Before evaluation:" << std::endl;
        print_branch(std::cout, branch);

        evaluate_branch(branch);

        std::cout << std::endl;
        std::cout << "-- After evaluation:" << std::endl;
        print_branch(std::cout, branch);

        std::cout << std::endl;
        std::cout << "-- Code result:" << std::endl;
        std::cout << get_branch_source_text(branch) << std::endl;

        return 0;
    }
    #endif

    // Generate cpp headers
    if (string_eq(args->get(0), "-gh")) {
        Branch branch;
        load_script(&branch, as_cstring(args->get(1)));

        if (has_static_errors(&branch)) {
            print_static_errors_formatted(&branch, std::cout);
            return 1;
        }

        std::cout << generate_cpp_headers(&branch);

        return 0;
    }

    // Run file checker
    if (string_eq(args->get(0), "-check"))
        return run_file_checker(as_cstring(args->get(1)));

    // Export parsed information
    if (string_eq(args->get(0), "-export")) {
        const char* filename = "";
        const char* format = "";
        if (args->length() >= 2)
            format = as_cstring(args->get(1));
        if (args->length() >= 3)
            filename = as_cstring(args->get(2));
        return run_exporting_parser(format, filename);
    }

    // Build tool
    if (string_eq(args->get(0), "-build")) {

        return run_build_tool(args);
    }

    // Stress test parser
    if (string_eq(args->get(0), "-parse100")) {

        const char* filename = as_cstring(args->get(1));

        for (int i=0; i < 100; i++) {
            Branch branch;
            load_script(&branch, filename);
            evaluate_branch(&branch);
        }
        return true;
    }
    if (string_eq(args->get(0), "-parse1000")) {

        const char* filename = as_cstring(args->get(1));

        for (int i=0; i < 1000; i++) {
            Branch branch;
            load_script(&branch, filename);
            evaluate_branch(&branch);
        }
        return true;
    }

    // C++ gen
    if (string_eq(args->get(0), "-cppgen")) {
        TValue remainingArgs;
        list_slice(args, 1, -1, &remainingArgs);
        run_generate_cpp(&remainingArgs);
        return 0;
    }

    // Command reader (from stdin)
    if (string_eq(args->get(0), "-run-stdin")) {
        run_commands_from_stdin();
        return 0;
    }

    // Default behavior with no flags: load args[0] as a script and run it.
    Branch* main_branch = create_branch(kernel());
    load_script(main_branch, as_cstring(args->get(0)));

    if (printRawWithProps)
        print_branch_with_properties(std::cout, main_branch);
    else if (printRaw)
        print_branch(std::cout, main_branch);

    if (printSource)
        std::cout << get_branch_source_text(main_branch);

    if (has_static_errors(main_branch)) {
        print_static_errors_formatted(main_branch, std::cout);
        return 1;
    }

    if (dontRunScript)
        return 0;

    EvalContext context;

    // Push any extra command-line arguments to context.argumentList
    List* inputs = set_list(context.argumentList.append());

    for (int i=1; i < args->length(); i++)
        copy(args->get(i), inputs->append());

    evaluate_branch(&context, main_branch);

    if (printState)
        std::cout << context.state.toString() << std::endl;

    if (error_occurred(&context)) {
        std::cout << "Error occurred:\n";
        print_runtime_error_formatted(&context, std::cout);
        std::cout << std::endl;
        return 1;
    }

    return 0;
}

} // namespace circa

using namespace circa;

EXPORT int circa_run_command_line(int argc, const char* args[])
{
    List args_v;
    set_list(&args_v, 0);
    for (int i=1; i < argc; i++)
        set_string(list_append(&args_v), args[i]);

    return circa::run_command_line(&args_v);
}
