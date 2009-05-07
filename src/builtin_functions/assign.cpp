// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace assign_function {

    void evaluate(Term* caller)
    {
        Term* value = caller->input(0);
        Term* target = caller->input(1);

        /*if (!is_value(target)) {
            error_occured(caller, "assign() tried to modify a non-value");
            return;
        }*/

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
