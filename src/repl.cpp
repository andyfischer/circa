// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

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
        parser::compile(&replState, parser::statement_list, input);
        int newHead = replState.length();

        bool anyErrors = false;

        for (int i=previousHead; i < newHead; i++) {
            Term* result = replState[i];

            if (has_error(result)) {
                std::cout << "error: " << get_error_message(result) << std::endl;
                anyErrors = true;
                break;
            }

            evaluate_term(result);

            if (has_error(result)) {
                std::cout << "error: " << get_error_message(result) << std::endl;
                anyErrors = true;
                break;
            }
        }

        // if there were any errors, erase the most recent results
        if (anyErrors) {
            replState.shorten(previousHead);
            continue;
        }

        std::cout << to_string(replState[replState.length()-1]) << std::endl;

        if (displayRaw) {
            for (int i=previousHead; i < newHead; i++)
                std::cout << term_to_raw_string(replState[i]) << std::endl;
        }
    }
}

} // namespace circa
