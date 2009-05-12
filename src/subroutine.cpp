// Copyright 2008 Paul Hodge

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

void register_subroutine_type(Branch& kernel)
{
    SUBROUTINE_TYPE = create_compound_type(kernel, "Subroutine");
    as_type(SUBROUTINE_TYPE).toString = subroutine_t::to_string;
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

    assert(has_hidden_state(func)); // sanity check

    func.prependInput(BRANCH_TYPE, "#state");
}

void initialize_subroutine_state(Term* term, Branch& state)
{
    assert(is_subroutine(term->function));
    duplicate_branch(as_branch(term->function), state);
}

void subroutine_call_evaluate(Term* caller)
{
    Branch &branch = get_state_for_subroutine_call(caller);

    if (branch.length() == 0)
        initialize_subroutine_state(caller, branch);

    Function &sub = get_function_data(caller->function);

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

    /*
    Term* out = func.subroutineBranch[OUTPUT_PLACEHOLDER_NAME];

    Branch& subBranch = create_branch(&branch);

    generate_feedback(subBranch, out, desired);
    */

    // TODO: feedback from outputs to outside this call
}

Branch& get_state_for_subroutine_call(Term* term)
{
    return as_branch(term->input(0));
}

} // namespace circa
