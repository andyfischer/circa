// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtins.h"
#include "types/list.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"

namespace circa {

Term* find_common_type(RefList const& list)
{
    if (list.length() == 0)
        return ANY_TYPE;

    bool all_equal = true;
    for (int i=1; i < list.length(); i++) {
        if (list[0] != list[i]) {
            all_equal = false;
            break;
        }
    }

    if (all_equal)
        return list[0];

    // Special case, allow ints to go into floats
    bool all_are_ints_or_floats = true;
    for (int i=0; i < list.length(); i++) {
        if ((list[i] != INT_TYPE) && (list[i] != FLOAT_TYPE)) {
            all_are_ints_or_floats = false;
            break;
        }
    }

    if (all_are_ints_or_floats)
        return FLOAT_TYPE;

    // Another special case, if all types are lists then use LIST_TYPE
    bool all_are_lists = true;
    for (int i=0; i < list.length(); i++) {
        if (!list_t::is_list_based_type(type_contents(list[i])))
            all_are_lists = false;
    }

    if (all_are_lists)
        return LIST_TYPE;

    // Otherwise give up
    return ANY_TYPE;
}

Term* find_type_of_get_index(Term* listTerm)
{
    if (listTerm->function == RANGE_FUNC)
        return INT_TYPE;

    if (listTerm->function == LIST_FUNC) {
        RefList inputTypes;
        for (int i=0; i < listTerm->numInputs(); i++)
            inputTypes.append(listTerm->input(i)->type);
        return find_common_type(inputTypes);
    }

    if (listTerm->function == COPY_FUNC)
        return find_type_of_get_index(listTerm->input(0));

    if (list_t::is_list_based_type(type_contents(listTerm->type))) {
        Branch& prototype = type_t::get_prototype(type_contents(listTerm->type));
        RefList types;
        for (int i=0; i < prototype.length(); i++)
            types.append(prototype[i]->type);
        return find_common_type(types);
    }

    // Unrecognized
    return ANY_TYPE;
}

}
