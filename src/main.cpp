// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "tests/all_tests.h"

using namespace circa;

int main(int nargs, const char * args[])
{
    initialize();

    if (nargs == 1) {
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
            evaluate_branch(branch);
            delete branch;
        }

    } catch (errors::CircaError &err)
    {
        std::cout << "Top level error:\n";
        std::cout << err.message() << std::endl;
    }

    shutdown();
}
