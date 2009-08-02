// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

namespace subroutine_t {
    std::string to_string(Term* term)
    {
        Branch& branch = as_branch(term);

        std::stringstream result;

        result << "def " << function_t::get_name(term) << "(";

        bool first = true;
        int numInputs = function_t::num_inputs(term);
        for (int i=0; i < numInputs; i++) {
            std::string name = function_t::get_input_name(term, i);

            if (name == "#state")
                continue;

            if (!first) result << ", ";
            first = false;
            result << function_t::get_input_type(term, i)->name;

            if (name != "" && name[0] != '#')
                result << " " << name;
        }

        result << ")";

        if (function_t::get_output_type(term) != VOID_TYPE) {
            result << term->stringPropOptional("syntaxHints:whitespacePreColon", "");
            result << ":";
            result << term->stringPropOptional("syntaxHints:whitespacePostColon", " ");
            result << function_t::get_output_type(term)->name;
        }

        result << term->stringPropOptional("syntaxHints:postHeadingWs", "\n");
        result << get_branch_source(branch);
        result << term->stringPropOptional("syntaxHints:preEndWs", "");
        result << "end";

        return result.str();
    }
}

bool is_subroutine(Term* term)
{
    return (term->type == FUNCTION_TYPE)
        && function_t::get_evaluate(term) == subroutine_call_evaluate;
}

void subroutine_update_hidden_state_type(Term* func)
{
    Branch& contents = as_branch(func);
    bool hasState = false;
    for (int i=0; i < contents.length(); i++) {
        if (contents[i] == NULL)
            continue;
        if (is_stateful(contents[i]))
            hasState = true;
        if (is_subroutine(contents[i]->function))
            if (is_function_stateful(contents[i]->function))
                hasState = true;
    }

    if (hasState) {
        function_t::get_hidden_state_type(func) = BRANCH_TYPE;
        bool alreadyHasStateInput = (function_t::num_inputs(func) > 0)
            && (function_t::get_input_name(func, 0) == "#state");
        if (!alreadyHasStateInput) {
            // Insert an input for state
            contents.insert(1, new Term());
            rewrite(contents[1], INPUT_PLACEHOLDER_FUNC, RefList());
            contents.bindName(contents[1], "#state");
        }
    } else {
        function_t::get_hidden_state_type(func) = VOID_TYPE;
    }
}

void subroutine_call_evaluate(Term* caller)
{
    Term* hiddenState = get_hidden_state_for_call(caller);

    if (hiddenState != NULL && !is_subroutine_state_expanded(hiddenState))
        expand_subroutines_hidden_state(caller, hiddenState);

    Term* function = hiddenState == NULL ? (Term*) caller->function : hiddenState;
    Branch& functionBranch = as_branch(function);

    int num_inputs = function_t::num_inputs(caller->function);

    if (num_inputs != caller->inputs.length()) {
        std::stringstream msg;
        msg << "Wrong number of inputs, expected: " << num_inputs
            << ", found: " << caller->inputs.length();
        error_occurred(caller, msg.str());
        return;
    }

    // Implant inputs
    for (int input=0; input < num_inputs; input++) {

        std::string inputName = function_t::get_input_name(function, input);
        if (inputName == "#state") {
            assert(input == 0);
            continue;
        }

        Term* term = function_t::get_input_placeholder(function, input);

        assert(term->function == INPUT_PLACEHOLDER_FUNC);

        assign_value(caller->inputs[input], term);
    }

    evaluate_branch(functionBranch);

    // Copy output
    if (functionBranch.length() > 0) {
        Term* output = functionBranch[functionBranch.length()-1];
        assert(output->name == "#out");
        if (output->type != VOID_TYPE)
            assign_value(output, caller);
    }
}

bool is_subroutine_state_expanded(Term* term)
{
    assert(term != NULL);
    return as_branch(term).length() > 0;
}

void expand_subroutines_hidden_state(Term* call, Term* state)
{
    assert(is_subroutine(call->function));
    assert(state != NULL);
    duplicate_branch(as_branch(call->function), as_branch(state));
}

} // namespace circa
