// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

using namespace circa;

int circa_main(std::vector<std::string> args)
{
    initialize();

    args.erase(args.begin());

    if (args.size() == 0) {
        run_all_tests();
        shutdown();
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
        Term* result = workspace.eval(command.str());
        std::cout << result->toString() << std::endl;
        shutdown();
        return 0;
    }

    if (args[0] == "--list-tests") {
        std::vector<std::string> testNames = list_all_test_names();

        std::vector<std::string>::const_iterator it;
        for (it = testNames.begin(); it != testNames.end(); ++it) {
            std::cout << *it << std::endl;
        }
        shutdown();
        return 0;
    }

    // Show compiled code
    if (args[0] == "-p") {
        Branch branch;
        parse_file(branch, args[1]);
        std::cout << branch_to_string_raw(branch);
        return 0;
    }

    // Reproduce source
    if (args[0] == "-s") {
        Branch branch;
        parse_file(branch, args[1]);
        std::cout << get_branch_source(branch) << std::endl;
        return 0;
    }

    // Do a feedback test
    if (args[0] == "-f") {
        Branch branch;
        parse_file(branch, args[1]);

        Branch &trainable_names = branch["_trainable"]->asBranch();
        for (int i=0; i < trainable_names.length(); i++)
            set_trainable(branch[trainable_names[i]->asString()], true);
        refresh_training_branch(branch);

        std::cout << std::endl;
        std::cout << "-- Before evaluation:" << std::endl;
        std::cout << branch_to_string_raw(branch);

        evaluate_branch(branch);

        std::cout << std::endl;
        std::cout << "-- After evaluation:" << std::endl;
        std::cout << get_branch_source(branch) << std::endl;

        return 0;
    }

    // Otherwise, run script
    Branch main_branch;
    parse_file(main_branch, args[0]);

    if (count_compile_errors(main_branch) > 0) {
        int count = count_compile_errors(main_branch);
        std::cout << count << " compile error";
        if (count != 1) std::cout << "s";
        std::cout << ":" << std::endl;
        print_compile_errors(main_branch, std::cout);
        return 1;
    } else {

        Term* error_listener = new Term();

        evaluate_branch(main_branch, error_listener);

        if (error_listener->hasError) {
            std::cout << "Error occured: " << error_listener->getErrorMessage() << std::endl;
            return 1;
        }
    }

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
