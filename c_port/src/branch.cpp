
#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "globals.h"
#include "operations.h"
#include "term.h"

void Branch::append(Term* term)
{
    terms.push_back(term);
}

bool Branch::containsName(string name)
{
    return names.contains(name);
}

Term* Branch::getNamed(string name)
{
    return names[name];
}

void Branch::bindName(Term* term, string name)
{
    names[name] = term;
}

Branch* as_branch(Term* term)
{
    if (term->type != BUILTIN_BRANCH_TYPE)
        throw errors::InternalTypeError(term, BUILTIN_BRANCH_TYPE);

    return (Branch*) term->value;
}

void Branch_alloc(Term* caller)
{
    caller->value = new Branch();
}

void Branch_copy(Term* source, Term* dest)
{
    // Todo
}

void branch_bind_name(Term* caller)
{
    // Recycles input 0
    as_branch(caller)->bindName(caller->inputs[2], as_string(caller->inputs[1]));
}

void branch_apply_function(Term* caller)
{
    // Recycles input 0
    Branch* branch = as_branch(caller);
    Term* function = caller->inputs[1];
    TermList* inputs = as_term_list(caller->inputs[2]);

    apply_function(branch, function, *inputs);
}

void initialize_branch(Branch* kernel)
{
    BUILTIN_BRANCH_TYPE = quick_create_type(kernel, "Branch", Branch_alloc, NULL, NULL);

    Term* bind_name = quick_create_function(kernel, "bind-name", branch_bind_name,
        TermList(BUILTIN_BRANCH_TYPE, get_global("string"), get_global("any")),
        BUILTIN_BRANCH_TYPE);
    as_function(bind_name)->recycleInput = 0;

    Term* apply_function = quick_create_function(kernel, "apply-function",
        branch_apply_function,
        TermList(BUILTIN_BRANCH_TYPE, get_global("Reference"), get_global("TermList")),
        BUILTIN_BRANCH_TYPE);
    as_function(apply_function)->recycleInput = 0;
}
