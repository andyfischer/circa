// Copyright 2008 Paul Hodge

#include "circa.h"
#include "values.h"

namespace circa {

static const char * INPUT_PLACEHOLDER_PREFIX = "#input-";
static const char * OUTPUT_PLACEHOLDER_NAME = "#output";

Subroutine::Subroutine()
  : branch(NULL)
{
}

Subroutine* as_subroutine(Term* term)
{
    if (!is_subroutine(term))
        throw errors::TypeError(term, SUBROUTINE_TYPE);

    return (Subroutine*) term->value;
}

bool is_subroutine(Term* term)
{
    return term->type == SUBROUTINE_TYPE;
}

void Subroutine_alloc(Term* term)
{
    term->value = new Subroutine();
    as_subroutine(term)->branch = new Branch();
}

void Subroutine_dealloc(Term* term)
{
    delete as_subroutine(term)->branch;
    delete as_subroutine(term);
}

void Subroutine_duplicate(Term* source, Term* dest)
{
    Subroutine* sourceSub = as_subroutine(source);
    Subroutine* destSub = as_subroutine(dest);
    destSub->branch->clear();

    *((Function*) destSub) = *((Function*) sourceSub);
    duplicate_branch(sourceSub->branch, destSub->branch);
}

std::string GetInputPlaceholderName(int index)
{
    std::stringstream sstream;
    sstream << INPUT_PLACEHOLDER_PREFIX << index;
    return sstream.str();
}

Branch* Subroutine_openBranch(Term* caller)
{
    Subroutine* sub = as_subroutine(caller->function);
    Branch* branch = as_branch(caller->state);

    // Copy inputs to input placeholders
    int numInputs = caller->inputs.count();
    for (int index=0; index < numInputs; index++) {
        Term* incomingInput = caller->inputs[index];
        std::string name = GetInputPlaceholderName(index);
        if (!branch->containsName(name)) {
            throw errors::InternalError(string("Too many arguments for subroutine ")
                + sub->name);
        }

        duplicate_value(incomingInput, branch->getNamed(name));
    }

    return branch;
}

void Subroutine_closeBranch(Term* caller)
{
    Subroutine* sub = as_subroutine(caller->function);
    Branch* branch = as_branch(caller->state);

    // Copy output to output placeholder, if one exists
    if (branch->containsName(OUTPUT_PLACEHOLDER_NAME)) {
        Term* outputPlaceholder = branch->getNamed(OUTPUT_PLACEHOLDER_NAME);
        steal_value(outputPlaceholder, caller);
    }
    else {
        // Not having an output is OK, but make sure we are declared with
        // type void.
        if (!(caller->type == VOID_TYPE)) {
            std::cout << "Warning: Inconsistent data. Subroutine " <<
                sub->name << " has a non-void output type, but has no output placeholder."
                << std::endl;
        }
    }
}

void subroutine_call__initialize(Term* caller)
{
    Subroutine *sub = as_subroutine(caller->function);
    Branch *stateBranch = as_branch(caller->state);
    duplicate_branch(sub->branch, stateBranch);

}

void subroutine_call__evaluate(Term* caller)
{
    Branch* branch = as_branch(caller->state);

    Subroutine_openBranch(caller);
    evaluate_branch(branch);
    Subroutine_closeBranch(caller);
}

void subroutine_create__evaluate(Term* caller)
{
    // 0: name (string)
    // 1: inputTypes (list of type)
    // 2: outputType (type)

    as_string(caller->inputs[0]);
    as_list(caller->inputs[1]);
    as_type(caller->inputs[2]);

    Subroutine* sub = as_subroutine(caller);
    sub->name = as_string(caller->inputs[0]);
    sub->initialize = subroutine_call__initialize;
    sub->evaluate = subroutine_call__evaluate;
    sub->inputTypes = as_list(caller->inputs[1]);
    sub->outputType = caller->inputs[2];
    sub->stateType = BRANCH_TYPE;

    // Create input placeholders
    for (int index=0; index < sub->inputTypes.count(); index++) {
        std::string name = GetInputPlaceholderName(index);
        Term* placeholder = create_constant(sub->branch, sub->inputTypes[index]);
        sub->branch->bindName(placeholder, name);
    }
}

void subroutine_name_input__evaluate(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    int index = as_int(caller->inputs[1]);
    std::string name = as_string(caller->inputs[2]);
    Subroutine* sub = as_subroutine(caller);
    Term* inputPlaceholder = sub->branch->getNamed(GetInputPlaceholderName(index));
    sub->branch->bindName(inputPlaceholder, name);
}

void subroutine_name_inputs__evaluate(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    TermList& name_list = as_list(caller->inputs[1]);
    Subroutine* sub = as_subroutine(caller);
    Branch* branch = sub->branch;

    for (int index=0; index < sub->inputTypes.count(); index++) {
        Term* inputPlaceholder = branch->getNamed(GetInputPlaceholderName(index));
        std::string newName = as_string(name_list.get(index));
        branch->bindName(inputPlaceholder, newName);
    }
}

void subroutine_get_branch__evaluate(Term* caller)
{
    Subroutine* sub = as_subroutine(caller->inputs[0]);
    *as_branch(caller) = *sub->branch;
}

void subroutine_set_branch__evaluate(Term* caller)
{
    // Input 0: Subroutine
    // Input 1: Branch
    recycle_value(caller->inputs[0], caller);
    Subroutine* sub = as_subroutine(caller);
    *sub->branch = *as_branch(caller->inputs[1]);
}

void subroutine_get_local__evaluate(Term* caller)
{
    // Input 0: Subroutine
    // Input 1: String name
    // Output: Reference
    Subroutine* sub = as_subroutine(caller->inputs[0]);
    std::string name = as_string(caller->inputs[1]);

    as_ref(caller) = sub->branch->getNamed(name);
}

void subroutine_bind__evaluate(Term* caller)
{
    // Input 0: Subroutine
    // Input 1: Reference
    // Input 2: String name
    recycle_value(caller->inputs[0], caller);
    Subroutine* sub = as_subroutine(caller);
    Term* ref = as_ref(caller->inputs[1]);
    std::string name = as_string(caller->inputs[2]);

    sub->branch->bindName(ref, name);
}

void subroutine_print__evaluate(Term* caller)
{
    Subroutine *sub = as_subroutine(caller->inputs[0]);
    std::stringstream output;
    output << sub->name << "()" << std::endl;
    output << "{" << std::endl;

    for (int i=0; i < sub->branch->terms.count(); i++) {
        Term* term = sub->branch->terms[i];
        output << "  " << as_function(term->function)->name << "(";

        bool firstInput = true;
        for (int inputIndex=0; inputIndex < term->inputs.count(); inputIndex++) {
            if (!firstInput) output << ", ";
            output << term->inputs[inputIndex]->toString();
            firstInput = false;
        }
        
        output << ")" << std::endl;
    }
    output << "}" << std::endl;
    as_string(caller) = output.str();
}

void subroutine_eval__evaluate(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    Subroutine *sub = as_subroutine(caller);
    std::string s = as_string(caller->inputs[1]);

    apply_statement(sub->branch, s);
}

void initialize_subroutine(Branch* kernel)
{
    SUBROUTINE_TYPE = quick_create_type(kernel, "Subroutine",
            Subroutine_alloc,
            Subroutine_dealloc,
            Subroutine_duplicate);
    as_type(SUBROUTINE_TYPE)->parentType = FUNCTION_TYPE;

    quick_create_function(kernel, "subroutine-create",
        subroutine_create__evaluate,
        TermList(STRING_TYPE,LIST_TYPE,TYPE_TYPE),
        SUBROUTINE_TYPE);

    Term* name_input = quick_create_function(kernel,
        "subroutine-name-input", subroutine_name_input__evaluate,
        TermList(SUBROUTINE_TYPE, INT_TYPE, STRING_TYPE), SUBROUTINE_TYPE);

    Term* name_inputs = quick_create_function(kernel,
        "subroutine-name-inputs", subroutine_name_inputs__evaluate,
        TermList(SUBROUTINE_TYPE, LIST_TYPE), SUBROUTINE_TYPE);

    quick_create_function(kernel, "subroutine-get-branch", subroutine_get_branch__evaluate,
        TermList(SUBROUTINE_TYPE), BRANCH_TYPE);

    Term* set_branch = quick_create_function(kernel,
        "subroutine-set-branch", subroutine_set_branch__evaluate,
        TermList(SUBROUTINE_TYPE, BRANCH_TYPE), SUBROUTINE_TYPE);
    as_function(set_branch)->recycleInput = 0;

    quick_create_function(kernel, "subroutine-get-local",
        subroutine_get_local__evaluate,
        TermList(SUBROUTINE_TYPE, STRING_TYPE), REFERENCE_TYPE);

    Term* bind = quick_create_function(kernel, "subroutine-bind",
        subroutine_bind__evaluate,
        TermList(SUBROUTINE_TYPE, REFERENCE_TYPE, STRING_TYPE), SUBROUTINE_TYPE);
    as_function(bind)->recycleInput = 0;

    quick_create_function(kernel,
        "subroutine-eval", subroutine_eval__evaluate,
        TermList(SUBROUTINE_TYPE, STRING_TYPE), SUBROUTINE_TYPE);

    quick_create_function(kernel, "subroutine-print",
        subroutine_print__evaluate,
        TermList(SUBROUTINE_TYPE), STRING_TYPE);
}

} // namespace circa
