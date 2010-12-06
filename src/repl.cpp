// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

#include "branch.h"
#include "errors.h"
#include "evaluation.h"
#include "introspection.h"
#include "parser.h"
#include "term.h"

namespace circa {

void repl_evaluate_line(Branch& branch, std::string const& input, std::ostream& output)
{
    int previousHead = branch.length();
    parser::compile(&branch, parser::statement_list, input);
    int newHead = branch.length();

    bool anyErrors = false;

    for (int i=previousHead; i < newHead; i++) {
        Term* result = branch[i];

        if (has_static_error(result)) {
            output << "error: " << get_static_error_message(result) << std::endl;
            anyErrors = true;
            break;
        }

        EvalContext context;

        // FIXME: Should only reevaluate new terms
        evaluate_branch(&context, branch);

        if (context.errorOccurred) {
            output << "error: ";
            print_runtime_error_formatted(context, std::cout);
            anyErrors = true;
            break;
        }
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
        if (input == "/clear") {
            replState.clear();
            std::cout << "Cleared working area" << std::endl;
            continue;
        }

        if (input == "/help") {
            std::cout << "Special commands: /raw, /help, /exit" << std::endl;
            continue;
        }

        int previousHead = replState.length();
        repl_evaluate_line(replState, input, std::cout);

        if (displayRaw) {
            for (int i=previousHead; i < replState.length(); i++) {
                std::cout << get_term_to_string_extended(replState[i]) << std::endl;
                if (replState[i]->nestedContents.length() > 0)
                    print_branch_raw(std::cout, replState[i]->nestedContents);
            }
        }
    }
}

} // namespace circa
