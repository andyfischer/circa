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

void remove_referencer(Term* term, Ref* ref)
{
    std::vector<Ref*>::iterator it;
    for (it = term->refs.begin(); it != term->refs.end();) {
        if (*it == ref) {
            it = term->refs.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace circa
