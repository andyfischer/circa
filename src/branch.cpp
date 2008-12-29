// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "runtime.h"
#include "ref_map.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {

int DEBUG_CURRENTLY_INSIDE_BRANCH_DESTRUCTOR = 0;

Branch::~Branch()
{
    DEBUG_CURRENTLY_INSIDE_BRANCH_DESTRUCTOR++;

    // Create a map where all our terms go to NULL
    ReferenceMap deleteMap;
    
    std::vector<Term*>::iterator it;
    for (it = _terms.begin(); it != _terms.end(); ++it) {
        if (*it != NULL)
            deleteMap[*it] = NULL;
    }

    this->remapPointers(deleteMap);

    // dealloc_value on all non-types
    for (unsigned int i = 0; i < _terms.size(); i++)
    {
        Term *term = _terms[i];

        if (term == NULL)
            continue;

        assert_good_pointer(term);

        if (term->type != TYPE_TYPE)
            dealloc_value(term);
    }

    // delete everybody
    for (unsigned int i = 0; i < _terms.size(); i++)
    {
        Term *term = _terms[i];
        if (term == NULL)
            continue;

        assert_good_pointer(term);

        dealloc_value(term);
        term->owningBranch = NULL;
        delete term;
    }

    DEBUG_CURRENTLY_INSIDE_BRANCH_DESTRUCTOR--;
}

void Branch::append(Term* term)
{
    assert_good_pointer(term);
    assert(term->owningBranch == NULL);
    term->owningBranch = this;
    _terms.push_back(term);
}

void Branch::bindName(Term* term, std::string name)
{
    names.bind(term, name);

#if 0
    // enable this code when subroutine inputs can work properly
    if (term->name != "") {
        throw std::runtime_error(std::string("term already has name: ")+term->name);
    }
#endif

    term->name = name;
}

Term* Branch::findNamed(std::string const& name) const
{
    if (containsName(name))
        return getNamed(name);

    if (outerScope != NULL) {
        assert(outerScope->value != this);

        Branch& outerBranch = as_branch(outerScope);
        return outerBranch.findNamed(name);
    }

    return get_global(name);
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

void Branch::visitPointers(PointerVisitor& visitor)
{
    struct VisitPointerIfOutsideBranch : PointerVisitor
    {
        Branch *branch;
        PointerVisitor &visitor;

        VisitPointerIfOutsideBranch(Branch *_branch, PointerVisitor &_visitor)
            : branch(_branch), visitor(_visitor) {}

        virtual void visitPointer(Term* term)
        {
            if (term == NULL)
                return;
            if (term->owningBranch != branch)
                visitor.visitPointer(term);
        }
    };

    VisitPointerIfOutsideBranch myVisitor(this, visitor);

    std::vector<Term*>::iterator it;
    for (it = _terms.begin(); it != _terms.end(); ++it) {
        if (*it == NULL)
            continue;

        // Type& type = as_type((*it)->type);

        visit_pointers(*it, myVisitor);
    }
}

void
Branch::clear()
{
    _terms.clear();
    names.clear();
}

void
Branch::hosted_remap_pointers(Term* caller, ReferenceMap const& map)
{
    as_branch(caller).remapPointers(map);
}

void
Branch::hosted_visit_pointers(Term* caller, PointerVisitor& visitor)
{
    as_branch(caller).visitPointers(visitor);
}

void
Branch::hosted_update_owner(Term* caller)
{
    as_branch(caller).owningTerm = caller;
}

Branch& as_branch(Term* term)
{
    assert_type(term, BRANCH_TYPE);
    assert(term->value != NULL);
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
        if (as_type(term->type).remapPointers != NULL)
            as_type(term->type).remapPointers(term, newTermMap);
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
