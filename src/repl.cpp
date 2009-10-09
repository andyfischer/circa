// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void start_repl()
{
    Branch replState;

    while (true) {
        std::cout << "> ";

        std::string input;

        if (!std::getline(std::cin, input))
            break;

        if (input == "exit")
            break;

        if (input == "")
            continue;

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
        if (anyErrors)
            replState.shorten(previousHead);
        else
            std::cout << to_string(replState[replState.length()-1]) << std::endl;
    }
}

} // namespace circa
