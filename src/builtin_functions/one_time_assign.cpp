// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace one_time_assign_function {

    void evaluate(EvalContext*, Term* caller)
    {
        Term* assigned = caller->input(0);

        if (!as_bool(assigned)) {
            cast(caller->input(1), caller);
            set_bool(assigned, true);
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
