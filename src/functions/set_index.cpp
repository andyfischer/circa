// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace set_index_function {

    CA_FUNCTION(evaluate)
    {
        copy(INPUT(0), OUTPUT);
        touch(OUTPUT);
        int index = INT_INPUT(1);
        copy(INPUT(2), OUTPUT->getIndex(index));
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        SET_INDEX_FUNC = import_function(kernel, evaluate,
                "set_index(any, int, any) -> any");
        function_t::get_specialize_type(SET_INDEX_FUNC) = specializeType;
    }
}
}
