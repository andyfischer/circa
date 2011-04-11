// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace run_single_statement_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(run_single_statement, "run_single_statement(any code, int)")
    {
        Branch& branch = INPUT_TERM(0)->nestedContents;
        int index = INT_INPUT(1);

        // Find the nth statement in this branch
        for (int i=0; i < branch.length(); i++) {
            if (!is_statement(branch[i]) || is_comment(branch[i]))
                continue;

            if (index == 0) {
                evaluate_minimum(CONTEXT, branch[i]);
                break;
            }

            index -= 1;
        }

        set_null(OUTPUT);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
