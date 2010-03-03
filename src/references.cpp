// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void Ref::set(Term* target)
{
    if (t == target)
        return;

    Term* previousTarget = t;

    t = target;

    if (t != NULL)
        t->refCount++;

    if (previousTarget != NULL) {
        assert_good_pointer(previousTarget);
        previousTarget->refCount--;
        if (previousTarget->refCount == 0)
            delete_term(previousTarget);
    }
}

void Ref::remap_pointers(Term* term, ReferenceMap const& map)
{
    as_ref(term) = map.getRemapped(as_ref(term));
}

void delete_term(Term* term)
{
    assert_good_pointer(term);

    assert(term->refCount == 0);

    term->inputs.clear();
    term->type = NULL;
    term->function = NULL;

    delete term;
}

void remap_pointers(Term* term, ReferenceMap const& map)
{
    assert_good_pointer(term);

    // make sure this map doesn't try to remap NULL, because such a thing
    // would almost definitely lead to errors.
    assert(!map.contains(NULL));

    for (int i=0; i < term->numInputs(); i++)
        set_input(term, i, map.getRemapped(term->input(i)));

    term->function = map.getRemapped(term->function);

    // TODO, call changeType if our type is changed
    // This was implemented once, and it caused spurious crash bugs
    // Term* newType = map.getRemapped(term->type);
    
    if (is_branch(term))
        as_branch(term).remapPointers(map);

    Type::RemapPointersFunc remapPointers = type_t::get_remap_pointers_func(term->type);

    // Remap on value
    if ((term->value_data.ptr != NULL)
            && term->type != NULL
            && (remapPointers)) {

        remapPointers(term, map);
    }

    // Remap inside properties
    for (int i=0; i < term->properties.length(); i++)
        remap_pointers(term->properties[i], map);
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

void remap_pointers(Branch& branch, Term* original, Term* replacement)
{
    ReferenceMap map;
    map[original] = replacement;

    for (int i=0; i < branch.length(); i++) {
        if (branch[i] == NULL) continue;
        remap_pointers(branch[i], map);
    }
}

} // namespace circa
