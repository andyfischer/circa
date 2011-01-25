// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace one_time_assign_function {

    CA_FUNCTION(evaluate)
    {
        if (!BOOL_INPUT(0)) {
            cast(INPUT(1), unbox_type(CALLER->type), OUTPUT);
            set_bool(INPUT(0), true);
        }
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(1)->type;
    }

    void setup(Branch& kernel)
    {
        ONE_TIME_ASSIGN_FUNC = import_function(kernel, evaluate,
                "one_time_assign(state bool, any) -> any");
        function_t::get_specialize_type(ONE_TIME_ASSIGN_FUNC) = specializeType;
    }
}
}
