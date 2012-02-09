// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../branch.h"
#include "../evaluation.h"
#include "../introspection.h"
#include "../static_checking.h"

namespace circa {

static void load(Branch* branch, std::string const& filename)
{
    if (filename == "") {
        clear_branch(branch);
        return;
    }

    load_script(branch, filename.c_str());
    if (has_static_errors(branch))
        print_static_errors_formatted(branch, std::cout);
}

int run_debugger_repl(std::string const& filename)
{
    Branch branch;

    load(&branch, filename);

    while (true) {
        std::cout << "> ";

        std::string input;

        if (!std::getline(std::cin, input))
            break;
        if (input == "exit" || input == "/exit")
            break;
        if (input == "")
            continue;

        if (input == "p") {
            print_branch(std::cout, &branch);
            continue;
        }

        if (input == "e") {
            evaluate_branch(&branch);
            continue;
        }

        if (input == "c") {
            print_static_errors_formatted(&branch, std::cout);
            continue;
        }

        std::cout << "unrecognized command: " << input << std::endl;
    }

    return 0;
}

} // namespace circa
