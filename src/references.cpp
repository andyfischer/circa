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
        _t->refCount++;

    if (previousTarget != NULL) {
        previousTarget->refCount--;
        if (previousTarget->refCount == 0)
            delete_term(previousTarget);
    }
}

void Ref::remap_pointers(Term* term, ReferenceMap const& map)
{
    deref(term) = map.getRemapped(deref(term));
}

void delete_term(Term* term)
{
    assert_good_pointer(term);

    dealloc_value(term);

    assert(term->refCount == 0);

    term->inputs.clear();
    term->type = NULL;
    term->function = NULL;
    term->state = NULL;

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
