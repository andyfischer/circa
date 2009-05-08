// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace assign_function {

    void evaluate(Term* caller)
    {
        // Target is on left, value on right.
        // This is a little confusing b/c the C function is backwards
        Term* value = caller->input(1);
        Term* target = caller->input(0);

        if (!value_fits_type(value, target->type)) {
            error_occured(caller, "Tried to assign a " + value->type->name + " to a "
                    + target->type->name);
            return;
        }

        assign_value(value, target);
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, evaluate, "assign(any, any)");
        as_function(ASSIGN_FUNC).pureFunction = false;
    }
}
}
