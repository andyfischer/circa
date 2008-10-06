// Copyright 2008 Andrew Fischer

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
    // Create a map where all our terms go to NULL
    ReferenceMap deleteMap;
    
    std::vector<Term*>::iterator it;
    for (it = _terms.begin(); it != _terms.end(); ++it) {
        if (*it != NULL)
            deleteMap[*it] = NULL;
    }

    // Perform remapping
    for (it = _terms.begin(); it != _terms.end(); ++it) {
        if (*it != NULL)
            remap_pointers(*it, deleteMap);
    }

    // dealloc_value on everybody
    for (int i = (int) _terms.size() - 1; i >= 0; i--)
    {
        Term *term = _terms[i];
        if (term == NULL)
            continue;

        assert_good(term);
        dealloc_value(term);
    }

    // delete stuff in reverse order
    for (int i = (int) _terms.size() - 1; i >= 0; i--)
    {
        Term *term = _terms[i];

        if (term == NULL)
            continue;

        assert_good(term);
        term->owningBranch = NULL;
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
    names.remapPointers(map);
}

void
Branch::clear()
{
    _terms.clear();
    names.clear();
}

Branch* as_branch(Term* term)
{
    if (term->type != BRANCH_TYPE)
        throw errors::TypeError(term, BRANCH_TYPE);

    return (Branch*) term->value;
}

} // namespace circa
