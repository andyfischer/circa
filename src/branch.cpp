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
    std::vector<Term*> myTerms = this->terms;

    this->terms.clear();

    std::vector<Term*>::iterator it;
    for (it = myTerms.begin(); it != myTerms.end(); ++it) {
        Term *term = *it;
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
