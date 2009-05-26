// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

namespace subroutine_t {
    std::string to_string(Term* term)
    {
        Branch& branch = as_branch(term);
        Function& func = get_subroutines_function_def(term);

        std::stringstream result;

        result << "def " << func.name << "(";

        bool first = true;
        for (int i=0; i < func.numInputs(); i++) {
            std::string name = func.getInputProperties(i).name;

            if (name == "#state")
                continue;

            if (!first) result << ", ";
            first = false;
            result << func.inputType(i)->name;

            if (func.getInputProperties(i).name != "")
                result << " " << name;
        }

        result << ")";

        if (func.outputType != VOID_TYPE)
            result << " : " << func.outputType->name;

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
    return as_function(as_branch(term)[get_name_for_attribute("function-def")]);
}

void initialize_subroutine(Term* term)
{
    Branch& branch = as_branch(term);
    Function& func = get_subroutines_function_def(term);
    func.evaluate = subroutine_call_evaluate;

    for (int input=0; input < func.numInputs(); input++) {
        std::string name = func.getInputProperties(input).name;
        Term *placeholder = apply(&branch, INPUT_PLACEHOLDER_FUNC,
            RefList(), name);
        change_type(placeholder, func.inputType(input));
        source_set_hidden(placeholder, true);
    }

    func.hiddenStateType = BRANCH_TYPE;
    func.prependInput(BRANCH_TYPE, "#state");
}

void subroutine_call_evaluate(Term* caller)
{
    Term* hiddenState = get_state_for_subroutine_call(caller);

    if (!is_subroutine_state_expanded(hiddenState))
        expand_subroutines_hidden_state(caller, hiddenState);

    Branch& branch = as_branch(hiddenState);

    Function &sub = get_function_data(caller->function);

    if (sub.inputTypes.count() != caller->inputs.count()) {
        std::stringstream msg;
        msg << "Wrong number of inputs, expected: " << sub.inputTypes.count()
            << ", found: " << caller->inputs.count();
        error_occurred(caller, msg.str());
        return;
    }

    // Implant inputs
    for (unsigned int input=1; input < sub.inputTypes.count(); input++) {

        std::string inputName = sub.getInputProperties(input).name;
        Term* inputTerm = branch[inputName];
        assert(inputTerm != NULL);
        assign_value(caller->inputs[input], inputTerm);
    }

    evaluate_branch(branch);

    // Copy output
    if (branch.contains(OUTPUT_PLACEHOLDER_NAME)) {
        Term* outputPlaceholder = branch[OUTPUT_PLACEHOLDER_NAME];
        assert(outputPlaceholder != NULL);
        assign_value(outputPlaceholder, caller);
    }
}

Term* get_state_for_subroutine_call(Term* term)
{
    bool hasState = get_function_data(term->function).hiddenStateType != VOID_TYPE;

    if (!hasState)
        return NULL;

    return term->input(0);
}

bool is_subroutine_state_expanded(Term* term)
{
    return as_branch(term).length() > 0;
}

void expand_subroutines_hidden_state(Term* term, Term* state)
{
    assert(is_subroutine(term->function));
    assert(state != NULL);
    duplicate_branch(as_branch(term->function), as_branch(state));
}

} // namespace circa
