// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void repl_evaluate_line(Branch& branch, std::string const& input, std::ostream& output)
{
    int previousHead = branch.length();
    parser::compile(&branch, parser::statement_list, input);
    int newHead = branch.length();

    bool anyErrors = false;

    for (int i=previousHead; i < newHead; i++) {
        Term* result = branch[i];

        if (has_error(result)) {
            output << "error: " << get_error_message(result) << std::endl;
            anyErrors = true;
            break;
        }

        evaluate_term(result);

        if (has_error(result)) {
            output << "error: " << get_error_message(result) << std::endl;
            anyErrors = true;
            break;
        }
    }

    // if there were any errors, erase the most recent results
    if (anyErrors) {
        branch.shorten(previousHead);
        return;
    }

    // Print results of the last expression
    output << to_string(branch[branch.length()-1]) << std::endl;
}

void start_repl()
{
    Branch replState;
    bool displayRaw = false;

    while (true) {
        std::cout << "> ";

        std::string input;

        if (!std::getline(std::cin, input))
            break;

        if (input == "exit" || input == "/exit")
            break;

        if (input == "")
            continue;

        if (input == "/raw") {
            displayRaw = !displayRaw;
            if (displayRaw) std::cout << "Displaying raw output" << std::endl;
            else std::cout << "Not displaying raw output" << std::endl;
            continue;
        }

        if (input == "/help") {
            std::cout << "Special commands: /raw, /help, /exit" << std::endl;
            continue;
        }

        int previousHead = replState.length();
        repl_evaluate_line(replState, input, std::cout);

        if (displayRaw) {
            for (int i=previousHead; i < replState.length(); i++)
                std::cout << term_to_raw_string(replState[i]) << std::endl;
        }
    }
}

} // namespace circa
