// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtins.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"

namespace circa {

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

    // Unrecognized
    return ANY_TYPE;
}

}
