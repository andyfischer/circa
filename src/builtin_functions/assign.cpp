// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace assign_function {

    void evaluate(Term* caller)
    {
        Term* value = caller->input(0);
        Term* target = caller->input(1);

        if (value->type != target->type)
            specialize_type(target, value->type);

        assign_value(value, target);
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, evaluate, "function assign(any, any)");
        as_function(ASSIGN_FUNC).pureFunction = false;
    }
}
}
