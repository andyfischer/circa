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
#include "parser.h"
#include "source_repro.h"
#include "static_checking.h"
#include "string_type.h"
#include "testing.h"

#include "tools/build_tool.h"
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
        "  circa <filename>        : Evaluate the given Circa source file\n"
        "  circa -repl             : Start an interactive read-eval-print-loop\n"
        "  circa -e <expression>   : Evaluate an expression on the command line\n"
        "  circa -test             : Run unit tests\n"
        "  circa -test <name>      : Run unit test of a certain name\n"
        "  circa -list-tests       : List every unit test name\n"
        "  circa -p <filename>     : Show the raw display of a source file\n"
        "  circa -ep <filename>    : Evaluate a source file and then show raw display\n"
        "  circa -pp <filename>    : Like -p but also print term properties\n"
        "  circa -s <filename>     : Compile the source file and reproduce its source code\n"
        "  circa -check <filename> : Statically check the script for any errors\n"
        "  circa -build <dir>      : Rebuild a module using a build.ca file\n"
        << std::endl;
}

int run_command_line(List* args)
{
    // No arguments, run tests
    if (list_length(args) == 0) {
        run_all_tests();
        return 0;
    }

    // Check for prepend options
    if (string_eq(args->get(0), "-breakon")) {
        String name;
        string_append(&name, "$");
        string_append(&name, (String*) args->get(1));
        DEBUG_BREAK_ON_TERM = strdup(as_cstring(&name));

        list_remove_index(args, 0);
        list_remove_index(args, 0);
        std::cout << "breaking on creation of term: " << DEBUG_BREAK_ON_TERM << std::endl;
    }

    // Run unit tests
    if (string_eq(args->get(0), "-test")) {
        if (args->length() > 1)
            run_tests(as_cstring(args->get(1)));
        else
            run_all_tests();
        return 0;
    }

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
        TaggedValue* result = workspace.eval(as_cstring(&command));
        std::cout << result->toString() << std::endl;
        return 0;
    }

    if (string_eq(args->get(0), "-list-tests")) {
        std::vector<std::string> testNames = list_all_test_names();

        std::vector<std::string>::const_iterator it;
        for (it = testNames.begin(); it != testNames.end(); ++it) {
            std::cout << *it << std::endl;
        }
        return 0;
    }

    // Show compiled code
    if (string_eq(args->get(0), "-p")) {
        Branch branch;
        load_script(&branch, as_cstring(args->get(1)));
        print_branch(std::cout, &branch);
        return 0;
    }

    // Show compiled code then evaluate
    if (string_eq(args->get(0), "-pe")) {
        Branch branch;
        load_script(&branch, as_cstring(args->get(1)));

        print_branch(std::cout, &branch);

        evaluate_branch(&branch);
        return 0;
    }

    // Evaluate and show compiled code
    if (string_eq(args->get(0), "-ep")) {
        Branch branch;
        load_script(&branch, as_cstring(args->get(1)));

        evaluate_branch(&branch);

        print_branch(std::cout, &branch);
        return 0;
    }

    // Show compiled code with properties
    if (string_eq(args->get(0), "-pp")) {
        Branch branch;
        load_script(&branch, as_cstring(args->get(1)));
        print_branch_with_properties(std::cout, &branch);
        return 0;
    }

    // Reproduce source
    if (string_eq(args->get(0), "-s")) {
        Branch branch;
        load_script(&branch, as_cstring(args->get(1)));
        std::cout << get_branch_source_text(&branch);
        return 0;
    }

    // Start repl
    if (string_eq(args->get(0), "-repl"))
        return run_repl();

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
        const char* filename = "";
        if (args->length() >= 2)
            filename = as_cstring(args->get(1));
        return run_build_tool(filename);
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
        TaggedValue remainingArgs;
        list_slice(args, 1, -1, &remainingArgs);
        run_generate_cpp(&remainingArgs);
        return 0;
    }

    // Default behavior with no flags: load args[0] as a script and run it.
    Branch* main_branch = create_branch(kernel());
    load_script(main_branch, as_cstring(args->get(0)));

    if (has_static_errors(main_branch)) {
        print_static_errors_formatted(main_branch, std::cout);
        //dump(main_branch);
        return 1;
    } else {

        Term error_listener;

        EvalContext context;

        // Push any extra command-line arguments to context.argumentList
        List* inputs = set_list(context.argumentList.append());

        for (int i=1; i < args->length(); i++)
            copy(args->get(i), inputs->append());

        evaluate_branch(&context, main_branch);

        if (context.errorOccurred) {
            std::cout << "Error occurred:\n";
            print_runtime_error_formatted(context, std::cout);
            std::cout << std::endl;
            return 1;
        }
    }

    return 0;
}

} // namespace circa

using namespace circa;

export_func int circa_run_command_line(int argc, const char* args[])
{
    List args_v;
    set_list(&args_v, 0);
    for (int i=1; i < argc; i++)
        set_string(list_append(&args_v), args[i]);

    return circa::run_command_line(&args_v);
}
