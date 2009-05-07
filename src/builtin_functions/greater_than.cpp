// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace greater_than_function {

    void evaluate(Term* caller)
    {
        Term* input0 = caller->input(0);
        Term* input1 = caller->input(1);

        Type &type = as_type(input0->type);

        if (type.lessThan == NULL) {
            error_occured(caller, "greaterThan not defined");
            return;
        }

        as_bool(caller) = !type.lessThan(input0, input1);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "greater_than(any,any) : bool");
        as_function(main_func).pureFunction = true;
    }
}
}
