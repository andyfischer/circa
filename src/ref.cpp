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
        previousTarget->removeReferencer(this);
}

} // namespace circa
