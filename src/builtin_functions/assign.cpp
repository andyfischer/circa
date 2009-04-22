// Copyright 2008 Paul Hodge

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

        assign_value(value, target);
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, evaluate, "assign(any, any)");
        as_function(ASSIGN_FUNC).pureFunction = false;
    }
}
}
