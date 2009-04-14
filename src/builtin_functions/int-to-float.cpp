// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "circa.h"
#include "introspection.h"

namespace circa {
namespace int_to_float_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_int(caller->input(0));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function int_to_float(int) -> float");
        as_function(main_func).pureFunction = true;

        INT_TO_FLOAT_FUNC = main_func;
    }
}
}
