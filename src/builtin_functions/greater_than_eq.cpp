// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace greater_than_eq_function {

    void evaluate(Term* caller)
    {
        Term* input0 = caller->input(0);
        Term* input1 = caller->input(1);

        Type &type = as_type(input0->type);

        if (type.lessThan == NULL) {
            error_occured(caller, "lessThan not defined");
            return;
        }

        if (type.equals == NULL) {
            error_occured(caller, "equals not defined");
            return;
        }

        bool lessThan = type.lessThan(input0, input1);
        bool equals = type.equals(input0, input1);

        as_bool(caller) = (!lessThan) || equals;
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "greater_than_eq(any,any) : bool");
        as_function(main_func).pureFunction = true;
    }
}
}
