
#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "globals.h"
#include "operations.h"

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

void Branch_alloc(Term* type, Term* caller)
{
    caller->value = new Branch();
}

void Branch_copy(Term* source, Term* dest)
{

}

void branch_bind_name(Term* caller)
{
    // Recycles input 0
    as_branch(caller)->bindName(caller->inputs[2], as_string(caller->inputs[1]));
}

void initialize_branch(Branch* kernel)
{
    BUILTIN_BRANCH_TYPE = quick_create_type(kernel, "Branch", Branch_alloc, NULL, Branch_copy);

    Term* bind_name = quick_create_function(kernel, "bind-name", branch_bind_name,
        TermList(BUILTIN_BRANCH_TYPE, get_global("string"), get_global("any")),
        BUILTIN_BRANCH_TYPE);
    as_function(bind_name)->recycleInput = 0;
}
