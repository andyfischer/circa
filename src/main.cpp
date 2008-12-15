// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

using namespace circa;

int circa_main(std::vector<std::string> args)
{
    initialize();

    args.erase(args.begin());

    bool justPrintBranch = false;
    bool justPrintSource = false;

    if (args.size() == 0) {
        run_all_tests();
        goto shutdown;
    }

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
        Term* result = eval_statement(workspace, command.str());
        std::cout << result->toString() << std::endl;
        goto shutdown;
    }

    if (args[0] == "--list-tests") {
        std::vector<std::string> testNames = list_all_test_names();

        std::vector<std::string>::const_iterator it;
        for (it = testNames.begin(); it != testNames.end(); ++it) {
            std::cout << *it << std::endl;
        }
        goto shutdown;
    }

    if (args[0] == "-p") {
        justPrintBranch = true;
        args.erase(args.begin());
    }

    if (args[0] == "-s") {
        justPrintSource = true;
        args.erase(args.begin());

        if (justPrintBranch) {
            // todo: fatal
        }
    }

    //try {
        Branch* branch = evaluate_file(args[0]);

        if (justPrintBranch) {
            print_branch_extended(*branch, std::cout);
        }

        else if (justPrintSource) {
            print_source(*branch, std::cout);
        }

        else if (has_compile_errors(*branch)) {
            print_compile_errors(*branch, std::cout);
        } else {
            evaluate_branch(*branch);
            print_runtime_errors(*branch, std::cout);
        }

        delete branch;

    /*} catch (std::runtime_error const& err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.what() << std::endl;
    }*/

shutdown:
    shutdown();
    
    return 0;
}

int main(int nargs, const char * args[])
{
    std::vector<std::string> args_vector;

    for (int i = 0; i < nargs; i++)
        args_vector.push_back(args[i]);

    return circa_main(args_vector);
}
