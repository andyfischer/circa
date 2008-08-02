
#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "subroutine.h"
#include "term.h"

Subroutine::Subroutine()
  : branch(NULL),
    outputPlaceholder(NULL)
{
}

Subroutine* as_subroutine(Term* term)
{
    if (!is_subroutine(term))
        throw errors::TypeError(term, BUILTIN_SUBROUTINE_TYPE);

    return (Subroutine*) term->value;
}

bool is_subroutine(Term* term)
{
    return term->type == BUILTIN_SUBROUTINE_TYPE;
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

void Subroutine_execute(Term* caller)
{
    Subroutine* sub = as_subroutine(caller->function);

    // Copy inputs to input placeholders
    for (int inputIndex=0; inputIndex < sub->inputTypes.count(); inputIndex++) {
        copy_term(caller->inputs[inputIndex], sub->inputPlaceholders[inputIndex]);
    }

    // Execute every term of ours
    execute_branch(sub->branch);

    // Copy output to output placeholder
    if (sub->outputPlaceholder != NULL)
        copy_term(sub->outputPlaceholder, caller);
}

void subroutine_create(Term* caller)
{
    // 0: name (string)
    // 1: inputTypes (list of type)
    // 2: outputType (type)

    as_string(caller->inputs[0]);
    as_list(caller->inputs[1]);
    as_type(caller->inputs[2]);

    Subroutine* sub = as_subroutine(caller);
    sub->name = as_string(caller->inputs[0]);
    sub->execute = Subroutine_execute;
    sub->inputTypes = *as_list(caller->inputs[1]);
    sub->outputType = caller->inputs[2];

    // Create input placeholders
    for (int inputIndex=0; inputIndex < sub->inputTypes.count(); inputIndex++) {
        sub->inputPlaceholders.setAt(inputIndex,
                create_constant(sub->branch, sub->inputTypes[inputIndex]));
    }
}

void subroutine_name_inputs(Term* caller)
{
    // Recycles input 0
    TermList* name_list = as_list(caller->inputs[1]);
    Subroutine* sub = as_subroutine(caller);
    for (int inputIndex=0; inputIndex < sub->inputPlaceholders.count(); inputIndex++) {
        sub->branch->bindName(sub->inputPlaceholders[inputIndex],
                as_string(name_list->get(inputIndex)));
    }
}

void subroutine_get_branch(Term* caller)
{
    Subroutine* sub = as_subroutine(caller->inputs[0]);
    *as_branch(caller) = *sub->branch;
}

void initialize_subroutine(Branch* kernel)
{
    BUILTIN_SUBROUTINE_TYPE = quick_create_type(kernel, "Subroutine", Subroutine_alloc, NULL);

    quick_create_function(kernel, "subroutine-create", subroutine_create,
            TermList(get_global("string"),get_global("List"),get_global("Type")),
            BUILTIN_SUBROUTINE_TYPE);

    Term* name_inputs = quick_create_function(kernel,
            "subroutine-name-inputs", subroutine_name_inputs,
            TermList(BUILTIN_SUBROUTINE_TYPE, get_global("List")), BUILTIN_SUBROUTINE_TYPE);
    as_function(name_inputs)->recycleInput = 0;

    quick_create_function(kernel, "subroutine-get-branch", subroutine_get_branch,
            TermList(BUILTIN_SUBROUTINE_TYPE), get_global("Branch"));
}
