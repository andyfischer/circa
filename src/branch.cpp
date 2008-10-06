// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "ref_map.h"
#include "term.h"
#include "values.h"

namespace circa {

Branch::~Branch()
{
    std::vector<Term*> myTerms = this->terms;

    this->terms.clear();

    // dealloc_value on everybody
    for (unsigned int i = myTerms.size() - 1; i >= 0; i--)
    {
        Term *term = myTerms[i];
        if (term == NULL)
            continue;

        assert_good(term);
        dealloc_value(term);
    }

    // delete stuff in reverse order
    for (unsigned int i = myTerms.size() - 1; i >= 0; i--)
    {
        Term *term = myTerms[i];

        if (term == NULL)
            continue;

        assert_good(term);
        delete term;
    }
}

Term* Branch::findNamed(std::string const& name) const
{
    if (containsName(name))
        return getNamed(name);

    return get_global(name);
}

void Branch::remapPointers(ReferenceMap const& map)
{
    for (unsigned int i=0; i < terms.size(); i++)
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

} // namespace circa
