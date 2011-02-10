// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "debug_valid_objects.h"

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
        assert_valid_term(previousTarget);
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
    assert_valid_term(term);

    ca_assert(term->refCount == 0);

    term->inputs.clear();
    term->type = NULL;
    term->function = NULL;

    dealloc_term(term);
}

void remap_pointers(Term* term, ReferenceMap const& map)
{
    assert_valid_term(term);

    // make sure this map doesn't try to remap NULL, because such a thing
    // would almost definitely lead to errors.
    ca_assert(!map.contains(NULL));

    for (int i=0; i < term->numInputs(); i++)
        set_input2(term, i, map.getRemapped(term->input(i)), term->inputs[i].outputIndex);

    term->function = map.getRemapped(term->function);

    // TODO, call changeType if our type is changed
    // This was implemented once, and it caused spurious crash bugs
    // Term* newType = map.getRemapped(term->type);
    
    Type::RemapPointers remapPointers = type_t::get_remap_pointers_func(term->type);

    // Remap on value
    if ((term->value_data.ptr != NULL)
            && term->type != NULL
            && (remapPointers)) {

        remapPointers(term, map);
    }

    // This code once called remap on term->properties

    // Remap inside nestedContents
    term->nestedContents.remapPointers(map);
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    assert_valid_term(term);
    assert_valid_term(original);
    ca_assert(original != NULL);

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
