// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "ref_map.h"
#include "term.h"
#include "type.h"
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

    remapPointers(deleteMap);

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

void Branch::append(Term* term)
{
    assert_good(term);
    assert(term->owningBranch == NULL);
    term->owningBranch = this;
    _terms.push_back(term);
}


Term* Branch::findNamed(std::string const& name) const
{
    if (containsName(name))
        return getNamed(name);

    return get_global(name);
}

void Branch::termDeleted(Term* term)
{
    ReferenceMap deleteMap;
    deleteMap[term] = NULL;

    remapPointers(deleteMap);

    for (int i = 0; i < (int) _terms.size(); i++) {
        if (_terms[i] == term)
            _terms[i] = NULL;
    }
}

void Branch::remapPointers(ReferenceMap const& map)
{
    names.remapPointers(map);

    std::vector<Term*>::iterator it;
    for (it = _terms.begin(); it != _terms.end(); ++it) {
        if (*it != NULL)
            remap_pointers(*it, map);
    }
}

void
Branch::clear()
{
    _terms.clear();
    names.clear();
}

Branch& as_branch(Term* term)
{
    assert_type(term, BRANCH_TYPE);

    return *((Branch*) term->value);
}

void duplicate_branch(Branch* source, Branch* dest)
{
    ReferenceMap newTermMap;

    // Duplicate every term
    for (int index=0; index < source->numTerms(); index++) {
        Term* source_term = source->get(index);

        Term* dest_term = create_term(dest, source_term->function, source_term->inputs);
        newTermMap[source_term] = dest_term;

        duplicate_value(source_term, dest_term);
    }

    // Remap terms
    for (int index=0; index < dest->numTerms(); index++) {
        Term* term = dest->get(index);
        term->inputs.remapPointers(newTermMap);
        if (as_type(term->type)->remapPointers != NULL)
            as_type(term->type)->remapPointers(term, newTermMap);
    }

    // Copy names
    TermNamespace::StringToTermMap::iterator it;
    for (it = source->names.begin(); it != source->names.end(); ++it) {
        std::string name = it->first;
        Term* original_term = it->second;
        dest->bindName(newTermMap.getRemapped(original_term), name);
    }
}

} // namespace circa
