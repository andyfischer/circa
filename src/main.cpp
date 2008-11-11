// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

using namespace circa;

int main(int nargs, const char * args[])
{
    initialize();

    if (nargs == 1) {
        run_test("builtin_function_tests::test_math");
        run_all_tests();
    }

    if (nargs == 2 && std::string("--list-tests") == args[1]) {
        std::vector<std::string> testNames = list_all_test_names();

        std::vector<std::string>::const_iterator it;
        for (it = testNames.begin(); it != testNames.end(); ++it) {
            std::cout << *it << std::endl;
        }
    }

    try {
        if (nargs > 1) {
            Branch* branch = evaluate_file(args[1]);

            if (has_compile_errors(*branch)) {
                print_compile_errors(*branch, std::cout);
            } else {
                evaluate_branch(*branch);
                print_runtime_errors(*branch, std::cout);
            }

            delete branch;
        }

    } catch (std::runtime_error const& err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.what() << std::endl;
    }

    shutdown();
}
