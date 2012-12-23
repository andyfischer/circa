// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "interpreter.h"
#include "inspection.h"
#include "static_checking.h"

namespace circa {

static void load(Block* block, std::string const& filename)
{
    if (filename == "") {
        clear_block(block);
        return;
    }

    load_script(block, filename.c_str());
    if (has_static_errors(block))
        print_static_errors_formatted(block, std::cout);
}

int run_debugger_repl(std::string const& filename)
{
    Block block;

    load(&block, filename);

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
            print_block(&block, std::cout);
            continue;
        }

        if (input == "e") {
            Stack stack;
            evaluate_block(&stack, &block);
            continue;
        }

        if (input == "c") {
            print_static_errors_formatted(&block, std::cout);
            continue;
        }

        std::cout << "unrecognized command: " << input << std::endl;
    }

    return 0;
}

} // namespace circa
