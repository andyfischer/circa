// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

using namespace circa;

int circa_main(std::vector<std::string> args)
{
    initialize();

    // load args into kernel
    List *cl_args = new List();
    
    for (unsigned int i=0; i < args.size(); i++) {
        cl_args->append(args[i]);
    }

    create_value(KERNEL, "List", cl_args, "cl-args");

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
        Term* result = workspace.eval(command.str());
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

    try
    {
        Branch workspace;
        string_value(workspace, args[0], "filename");
        Branch& branch = as_branch(workspace.eval("evaluate-file(filename)"));

        if (justPrintBranch) {
            print_branch_extended(branch, std::cout);
        }

        else if (justPrintSource) {
            print_source(branch, std::cout);
        }

        else if (has_compile_errors(branch)) {
            print_compile_errors(branch, std::cout);
        } else {

            Term* error_listener = new Term();

            evaluate_branch(branch, error_listener);

            if (error_listener->hasError()) {
                std::cout << "Error occured: " << error_listener->getErrorMessage() << std::endl;
            }

            // print_runtime_errors(branch, std::cout);
        }

    } catch (std::runtime_error const& err)
    {
        std::cout << "Top level exception:\n";
        std::cout << err.what() << std::endl;
    }

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

