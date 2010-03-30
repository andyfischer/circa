// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace assign_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        // The thing we are changing is on the left, the desired value is on the right
        // This is a little confusing because the C function 'assign_value' is the other
        // way around. The reason we have this order is because the infix operator :=
        // arranges its inputs as target := desired.
        Term* target = caller->input(0);
        Term* value = caller->input(1);

        // If target is a Int, then we might have to change it to Float
        if (target->type == INT_TYPE && value->type == FLOAT_TYPE)
            change_type(target, FLOAT_TYPE);

        if (!value_fits_type(value, target->type)) {
            error_occurred(cxt, caller,
                    "Tried to assign a " + value->type->name + " to a "
                    + target->type->name);
            return;
        }

        cast(value, target);
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, evaluate, "assign(any, any)");
    }
}
}
