// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace unsafe_assign_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        // The thing we are changing is on the left, the desired value is on the right
        // This is a little confusing because the C function 'cast' is the other
        // way around. The reason we have this order is because the infix operator :=
        // arranges its inputs as target := source.
        Term* target = caller->input(0);
        Term* source = caller->input(1);

        if (!matches_type(declared_type(target), source)) {
            error_occurred(cxt, caller,
                    "Tried to assign a " + source->type->name + " to a "
                    + target->type->name);
            return;
        }

        cast(source, target);
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, evaluate, "unsafe_assign(any, any)");
    }
}
}
