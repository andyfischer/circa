// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace set_index_function {

    void evaluate(EvalContext*, Term* caller)
    {
        copy(caller->input(0), caller);

        int index = caller->input(1)->asInt();
        cast(caller->input(2), as_branch(caller)[index]);
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
