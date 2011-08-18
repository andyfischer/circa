// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"
#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "codegen.h"
#include "errors.h"
#include "evaluation.h"
#include "feedback.h"
#include "introspection.h"
#include "parser.h"
#include "source_repro.h"
#include "static_checking.h"
#include "testing.h"

#include "tools/debugger_repl.h"
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
        << std::endl;
}

int run_command_line(std::vector<std::string> args)
{
    // No arguments, run tests
    if (args.size() == 0) {
        run_all_tests();
        return 0;
    }

    // Run unit tests
    if (args[0] == "-test") {
        if (args.size() > 1)
            run_tests(args[1]);
        else
            run_all_tests();
        return 0;
    }

    // Print help
    if (args[0] == "-help") {
        print_usage();
        return 0;
    }

    // Eval mode
    if (args[0] == "-e") {
        args.erase(args.begin());
        std::stringstream command;
        bool firstArg = true;
        while (!args.empty()) {
            if (!firstArg) command << " ";
            command << args[0];
            args.erase(args.begin());
            firstArg = false;
        }

        Branch workspace;
        TaggedValue* result = workspace.eval(command.str());
        std::cout << result->toString() << std::endl;
        return 0;
    }

    if (args[0] == "-list-tests") {
        std::vector<std::string> testNames = list_all_test_names();

        std::vector<std::string>::const_iterator it;
        for (it = testNames.begin(); it != testNames.end(); ++it) {
            std::cout << *it << std::endl;
        }
        return 0;
    }

    // Show compiled code
    if (args[0] == "-p") {
        Branch branch;
        parse_script(branch, args[1]);
        print_branch(std::cout, branch);
        return 0;
    }

    // Evaluate and show compiled code
    if (args[0] == "-ep") {
        Branch branch;
        parse_script(branch, args[1]);

        evaluate_branch(branch);

        print_branch(std::cout, branch);
        return 0;
    }

    // Show compiled code with properties
    if (args[0] == "-pp") {
        Branch branch;
        parse_script(branch, args[1]);
        print_branch_with_properties(std::cout, branch);
        return 0;
    }

    // Reproduce source
    if (args[0] == "-s") {
        Branch branch;
        parse_script(branch, args[1]);
        std::cout << get_branch_source_text(branch) << std::endl;
        return 0;
    }

    // Start repl
    if (args[0] == "-repl")
        return run_repl();

    // Start debugger repl
    if (args[0] == "-d")
        return run_debugger_repl(args[1]);

    // Do a feedback test
    #if 0
    if (args[0] == "-f") {
        Branch branch;
        parse_script(branch, args[1]);

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
    if (args[0] == "-gh") {
        Branch branch;
        parse_script(branch, args[1]);

        if (has_static_errors(branch)) {
            print_static_errors_formatted(branch, std::cout);
            return 1;
        }

        std::cout << generate_cpp_headers(branch);

        return 0;
    }

    // Run file checker
    if (args[0] == "-check")
        return run_file_checker(args[1].c_str());

    // Otherwise, load args[0] as a script and run it
    Branch& main_branch = create_branch(kernel());
    parse_script(main_branch, args[0]);

    if (has_static_errors(main_branch)) {
        print_static_errors_formatted(main_branch, std::cout);
        return 1;
    } else {

        Term error_listener;

        EvalContext context;

        // Push any extra command-line arguments to context.inputStack
        List* inputs = set_list(context.inputStack.append());

        for (size_t i=1; i < args.size(); i++)
            set_string(inputs->append(), args[i]);

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

export_func int circa_run_command_line(int argc, const char* args[])
{
    std::vector<std::string> argv;

    for (int i = 1; i < argc; i++)
        argv.push_back(args[i]);

    return circa::run_command_line(argv);
}
