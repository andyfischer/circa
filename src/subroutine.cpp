// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

bool is_subroutine(Term* term)
{
    return is_function(term) && as_function(term).subroutineBranch.numTerms() > 0;
}

void initialize_as_subroutine(Function& func)
{
    func.evaluate = Function::subroutine_call_evaluate;
    func.stateType = BRANCH_TYPE;

    for (int input=0; input < func.numInputs(); input++) {
        Term *placeholder = create_value(&func.subroutineBranch,
                func.inputType(input), func.getInputProperties(input).name);
        source_set_hidden(placeholder, true);
    }

    func.prependInput(BRANCH_TYPE, "#state");
}

void initialize_subroutine_state(Term* term, Branch& state)
{
    Function &def = as_function(term->function);
    state.clear();
    duplicate_branch(def.subroutineBranch, state);
}

void
Function::subroutine_call_evaluate(Term* caller)
{
    Branch &branch = get_state_for_subroutine_call(caller);

    if (branch.numTerms() == 0)
        initialize_subroutine_state(caller, branch);

    Function &sub = as_function(caller->function);

    if (sub.inputTypes.count() != caller->inputs.count()) {
        std::stringstream msg;
        msg << "Wrong number of inputs, expected: " << sub.inputTypes.count()
            << ", found: " << caller->inputs.count();
        error_occured(caller, msg.str());
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

Branch& get_state_for_subroutine_call(Term* term)
{
    return as_branch(term->input(0));
}

} // namespace circa
