
#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "term.h"

namespace circa {

Branch::Branch()
{
}

void Branch::append(Term* term)
{
    this->terms.append(term);
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
    names.bind(term, name);
}

void Branch::remapPointers(TermMap const& map)
{
    terms.remapPointers(map);
    names.remapPointers(map);
}

void
Branch::clear()
{
    terms.clear();
    names.clear();
}

Branch* as_branch(Term* term)
{
    if (term->type != BRANCH_TYPE)
        throw errors::TypeError(term, BRANCH_TYPE);

    return (Branch*) term->value;
}

void Branch_alloc(Term* caller)
{
    caller->value = new Branch();
}

void Branch_dealloc(Term* caller)
{
    delete as_branch(caller);
}

void Branch_copy(Term* source, Term* dest)
{
    // Todo
}

void branch_bind_name(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    as_branch(caller)->bindName(caller->inputs[2], as_string(caller->inputs[1]));
}

void initialize_branch(Branch* kernel)
{
    BRANCH_TYPE = quick_create_type(kernel, "Branch",
            Branch_alloc,
            Branch_dealloc,
            Branch_copy);

    Term* bind_name = quick_create_function(kernel, "bind-name", branch_bind_name,
        TermList(BRANCH_TYPE, get_global("string"), get_global("any")),
        BRANCH_TYPE);
    as_function(bind_name)->recycleInput = 0;
}

} // namespace circa
