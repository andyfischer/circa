// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

bool is_subroutine(Term* term)
{
    return is_function(term) && as_function(term).subroutineBranch.length() > 0;
}

void initialize_as_subroutine(Function& func)
{
    func.evaluate = Function::subroutine_call_evaluate;

    for (int input=0; input < func.numInputs(); input++) {
        std::string name = func.getInputProperties(input).name;
        Term *placeholder = apply(&func.subroutineBranch, INPUT_PLACEHOLDER_FUNC,
            RefList(), name);
        change_type(placeholder, func.inputType(input));
        source_set_hidden(placeholder, true);
    }

    func.hiddenStateType = BRANCH_TYPE;

    assert(has_hidden_state(func)); // sanity check

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

    if (branch.length() == 0)
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

void subroutine_feedback(Branch& branch, Term* subject, Term* desired)
{
    Function& func = as_function(subject);

    Term* out = func.subroutineBranch[OUTPUT_PLACEHOLDER_NAME];

    Branch& subBranch = create_branch(&branch);

    generate_feedback(subBranch, out, desired);

    // TODO: feedback from outputs to outside this call
}

Branch& get_state_for_subroutine_call(Term* term)
{
    return as_branch(term->input(0));
}

} // namespace circa
