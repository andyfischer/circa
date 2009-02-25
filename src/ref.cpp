// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

void Ref::set(Term* target)
{
    if (_target == target)
        return;

    Term* previousTarget = _target;

    _target = target;

    if (_target != NULL)
        _target->refs.push_back(this);

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

} // namespace circa
