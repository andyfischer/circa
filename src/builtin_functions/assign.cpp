// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace assign_function {

    void evaluate(Term* caller)
    {
        // The thing we are changing is on the left, the desired value is on the right
        // (This is a little confusing because the C function 'assign_value' is the other
        // way around.);
        Term* target = caller->input(0);
        Term* value = caller->input(1);

        // If target is a Int, then we might have to change it to Float
        if (target->type == INT_TYPE && value->type == FLOAT_TYPE)
            change_type(target, FLOAT_TYPE);

        if (!value_fits_type(value, target->type)) {
            error_occurred(caller, "Tried to assign a " + value->type->name + " to a "
                    + target->type->name);
            return;
        }

        assign_value(value, target);
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, evaluate, "assign(any, any)");
    }
}
}
