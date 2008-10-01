// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "term.h"
#include "values.h"

namespace circa {

Branch::~Branch()
{
    std::vector<Term*> myTerms = this->terms;

    this->terms.clear();

    std::vector<Term*>::iterator it;
    for (it = myTerms.begin(); it != myTerms.end(); ++it) {
        // FIXME
        // We should delete terms here, but it currently causes errors
        //delete *it;
        Term *term = *it;
        if (term != NULL)
            term->owningBranch = NULL;
    }
}

Term* Branch::findNamed(std::string const& name) const
{
    if (containsName(name))
        return getNamed(name);

    return get_global(name);
}

void Branch::remapPointers(TermMap const& map)
{
    for (int i=0; i < terms.size(); i++)
        terms[i] = map.getRemapped(terms[i]);

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

void branch_bind_name(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    as_branch(caller)->bindName(caller->inputs[2], as_string(caller->inputs[1]));
}

void initialize_branch(Branch* kernel)
{
    Term* bind_name = quick_create_function(kernel, "bind-name", branch_bind_name,
        ReferenceList(BRANCH_TYPE, STRING_TYPE, ANY_TYPE),
        BRANCH_TYPE);
}

} // namespace circa
