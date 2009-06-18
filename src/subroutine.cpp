// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

namespace subroutine_t {
    std::string to_string(Term* term)
    {
        Branch& branch = as_branch(term);

        std::stringstream result;

        result << "def " << function_get_name(term) << "(";

        bool first = true;
        int numInputs = function_num_inputs(term);
        for (int i=0; i < numInputs; i++) {
            std::string name = function_get_input_name(term, i);

            if (name == "#state")
                continue;

            if (!first) result << ", ";
            first = false;
            result << function_get_input_type(term, i)->name;

            if (name != "")
                result << " " << name;
        }

        result << ")";

        if (function_get_output_type(term) != VOID_TYPE) {
            result << term->stringPropOptional("syntaxHints:whitespacePreColon", " ");
            result << ":";
            result << term->stringPropOptional("syntaxHints:whitespacePostColon", " ");
            result << function_get_output_type(term)->name;
        }

        result << "\n";

        result << get_branch_source(branch);
        
        result << "end";

        return result.str();
    }
}

bool is_subroutine(Term* term)
{
    return term->type == SUBROUTINE_TYPE && (SUBROUTINE_TYPE != NULL);
}

Function& get_subroutines_function_def(Term* term)
{
    assert(is_subroutine(term));
    Term* def = as_branch(term)[0];
    test_assert(def->name == "#attr:function-def");
    return as_function(def);
}

void initialize_subroutine(Term* term)
{
    Branch& branch = as_branch(term);
    Function& func = get_subroutines_function_def(term);
    func.evaluate = subroutine_call_evaluate;

    int numInputs = function_num_inputs(term);
    for (int input=0; input < numInputs; input++) {
        std::string name = function_get_input_name(term, input);
        Term *placeholder = apply(&branch, INPUT_PLACEHOLDER_FUNC,
            RefList(), name);
        Term* type = function_get_input_type(term, input);
        change_type(placeholder, type);
        source_set_hidden(placeholder, true);
    }

    function_get_hidden_state_type(term) = VOID_TYPE;
}

void subroutine_update_hidden_state_type(Term* sub)
{
    Branch& contents = as_branch(sub);
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

    Function& func = get_function_data(sub);
    if (hasState) {
        function_get_hidden_state_type(sub) = BRANCH_TYPE;
        bool alreadyHasStateInput = (function_num_inputs(sub) > 0)
            && (function_get_input_name(sub, 0) == "#state");
        if (!alreadyHasStateInput)
            func.prependInput(BRANCH_TYPE, "#state");
    } else {
        function_get_hidden_state_type(sub) = VOID_TYPE;
    }
}

void subroutine_call_evaluate(Term* caller)
{
    Term* hiddenState = get_hidden_state_for_call(caller);

    if (hiddenState != NULL && !is_subroutine_state_expanded(hiddenState))
        expand_subroutines_hidden_state(caller, hiddenState);

    Branch& branch = (hiddenState == NULL) ?
                         as_branch(caller->function)
                         : as_branch(hiddenState);

    Function &sub = get_function_data(caller->function);

    if (sub.inputTypes.length() != caller->inputs.length()) {
        std::stringstream msg;
        msg << "Wrong number of inputs, expected: " << sub.inputTypes.length()
            << ", found: " << caller->inputs.length();
        error_occurred(caller, msg.str());
        return;
    }

    // Implant inputs
    int implantIndex = 1; // skip #attr:function-def
    for (int input=0; input < sub.inputTypes.length(); input++) {

        std::string inputName = sub.getInputProperties(input).name;
        if (inputName == "#state")
            continue;

        Term* term = branch[implantIndex++];

        assert(term->name == inputName);
        assert(term->function == INPUT_PLACEHOLDER_FUNC);

        assign_value(caller->inputs[input], term);
    }

    evaluate_branch(branch);

    // Copy output
    if (branch.length() > 0) {
        Term* output = branch[branch.length()-1];
        if (output->name == "#out")
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

bool sanity_check_subroutine(Term* sub, std::string& message)
{
    if (!is_subroutine(sub))
        return true;

    Branch& contents = as_branch(sub);

    // If the subroutine has an #out term, then it must be the last one
    if (contents.contains(OUTPUT_PLACEHOLDER_NAME)
            && contents[contents.length()-1]->name != OUTPUT_PLACEHOLDER_NAME) {

        message = "#out is bound, but the last term isn't named #out";
        return false;
    }

    return true;
}

} // namespace circa
