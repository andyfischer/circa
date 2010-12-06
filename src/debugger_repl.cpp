// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "branch.h"
#include "common_headers.h"
#include "errors.h"
#include "evaluation.h"
#include "introspection.h"

namespace circa {

static void load(Branch& branch, std::string const& filename)
{
    if (filename == "") {
        branch.clear();
        return;
    }

    parse_script(branch, filename);
    if (has_static_errors(branch))
        print_static_errors_formatted(branch, std::cout);
}

void start_debugger_repl(std::string const& filename)
{
    Branch branch;

    load(branch, filename);

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
            print_branch_raw(std::cout, branch);
            continue;
        }

        if (input == "e") {
            evaluate_branch(branch);
            continue;
        }

        if (input == "c") {
            print_static_errors_formatted(branch, std::cout);
            continue;
        }

        std::cout << "unrecognized command: " << input << std::endl;
    }
}

} // namespace circa
