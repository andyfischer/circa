// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

void Ref::set(Term* target)
{
    if (_t == target)
        return;

    Term* previousTarget = _t;

    _t = target;

    if (_t != NULL)
        _t->refs.push_back(this);

    if (previousTarget != NULL)
        remove_referencer(previousTarget, this);
}

void Ref::remap_pointers(Term* term, ReferenceMap const& map)
{
    as_ref(term) = map.getRemapped(as_ref(term));
}

class ReferenceIteratorForReferenceType : public ReferenceIterator
{
private:
    Term* _containingTerm;

public:
    ReferenceIteratorForReferenceType(Term* containingTerm)
      : _containingTerm(containingTerm)
    {
        if (_containingTerm->value == NULL)
            _containingTerm = NULL;
    }

    virtual Ref& current()
    {
        assert(!finished());
        return as_ref(_containingTerm);
    }
    virtual void advance()
    {
        _containingTerm = NULL;
    }
    virtual bool finished()
    {
        return _containingTerm == NULL;
    }
};

ReferenceIterator* Ref::start_reference_iterator(Term* term)
{
    return new ReferenceIteratorForReferenceType(term);
}

void remove_referencer(Term* term, Ref* ref)
{
    assert_good_pointer(term);

    std::vector<Ref*>::iterator it;
    for (it = term->refs.begin(); it != term->refs.end();) {
        if (*it == ref) {
            it = term->refs.erase(it);
        } else {
            ++it;
        }
    }

    if (term->refs.size() == 0)
        delete_term(term);
}

void delete_term(Term* term)
{
    assert_good_pointer(term);

    if (term->state != NULL)
        delete_term(term->state);
    term->state = NULL;

    dealloc_value(term);

    // Clear references
    std::vector<Ref*>::iterator it;
    for (it = term->refs.begin(); it != term->refs.end(); ++it) {
        (*it)->_t = NULL;
    }

    term->refs.clear();
    term->inputs.clear();
    term->type = NULL;
    term->function = NULL;

#if DEBUG_CHECK_FOR_BAD_POINTERS
    DEBUG_GOOD_POINTER_SET.erase(term);
#endif

#if !DEBUG_NEVER_DELETE_TERMS
    delete term;
#endif
}

void remap_pointers(Term* term, ReferenceMap const& map)
{
    assert_good_pointer(term);

    // make sure this map doesn't try to remap NULL, because such a thing
    // would almost definitely lead to errors.
    assert(!map.contains(NULL));

    term->inputs.remapPointers(map);
    term->function = map.getRemapped(term->function);

    if (map.getRemapped(term->type) != term->type)
        std::cout << "warn: in remap_pointers, remapping type is not yet supported" << std::endl;

    if ((term->value != NULL)
            && term->type != NULL
            && (as_type(term->type).remapPointers != NULL)) {

        as_type(term->type).remapPointers(term, map);
    }

    if (term->state != NULL)
        remap_pointers(term->state, map);
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    assert_good_pointer(term);
    assert_good_pointer(original);
    assert(original != NULL);

    ReferenceMap map;
    map[original] = replacement;
    remap_pointers(term, map);
}


} // namespace circa
