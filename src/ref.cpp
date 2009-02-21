// Copyright 2008 Paul Hodge

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

void Ref::visit_pointers(Term* term, PointerVisitor& visitor)
{
    visitor.visitPointer(as_ref(term));
}

void Ref::remap_pointers(Term* term, ReferenceMap const& map)
{
    as_ref(term) = map.getRemapped(as_ref(term));
}

class ReferencePointerIterator : public PointerIterator
{
private:
    Term* _containingTerm;

public:
    ReferencePointerIterator(Term* containingTerm)
      : _containingTerm(containingTerm)
    {
        if (_containingTerm->value == NULL)
            _containingTerm = NULL;
    }

    virtual Term* current()
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

PointerIterator* Ref::start_pointer_iterator(Term* term)
{
    return new ReferencePointerIterator(term);
}

} // namespace circa
