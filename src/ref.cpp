// Copyright 2008 Paul Hodge

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

} // namespace circa
