// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../branch.h"
#include "../kernel.h"
#include "../evaluation.h"
#include "../introspection.h"
#include "../parser.h"
#include "../static_checking.h"
#include "../term.h"

namespace circa {

void repl_evaluate_line(EvalContext* context, std::string const& input, std::ostream& output)
{
    Branch* branch = top_branch(context);
    int previousHead = branch->length();
    parser::compile(branch, parser::statement_list, input);
    int newHead = branch->length();

    bool anyErrors = false;

    int resultIndex = -1;

    for (int i=previousHead; i < newHead; i++) {
        Term* result = branch->get(i);

        if (has_static_error(result)) {
            output << "error: ";
            print_static_error(result, output);
            output << std::endl;
            anyErrors = true;
            break;
        }

        evaluate_single_term(context, result);

        if (error_occurred(context)) {
            output << "error: ";
            print_runtime_error_formatted(context, std::cout);
            anyErrors = true;
            break;
        }

        resultIndex = i;
    }

    // Print results of the last expression
    if (!anyErrors && resultIndex != -1) {
        Term* result = branch->get(resultIndex);
        if (result->type != as_type(VOID_TYPE)) {
            output << get_register(context, result)->toString() << std::endl;
        }
    }
}

int run_repl()
{
    EvalContext context;
    Branch branch;
    bool displayRaw = false;

    push_frame(&context, &branch);

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
            clear_branch(&branch);
            std::cout << "Cleared working area" << std::endl;
            continue;
        }
        if (input == "/dump") {
            dump(&branch);
            continue;
        }

        if (input == "/help") {
            std::cout << "Special commands: /raw, /help, /clear, /dump, /exit" << std::endl;
            continue;
        }

        int previousHead = branch.length();
        repl_evaluate_line(&context, input, std::cout);

        if (displayRaw) {
            for (int i=previousHead; i < branch.length(); i++) {
                std::cout << get_term_to_string_extended(branch[i]) << std::endl;
                if (nested_contents(branch[i])->length() > 0)
                    print_branch(std::cout, nested_contents(branch[i]));
            }
        }
    }

    return 0;
}

} // namespace circa
