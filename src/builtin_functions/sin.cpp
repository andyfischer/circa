// Copyright 2008 Andrew Fischer

#include "circa.h"

#include "math.h"

namespace circa {
namespace sin_function {

    void evaluate(Term* caller)
    {
        float input = as_float(caller->input(0));
        as_float(caller) = sin(input);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function sin(float) -> float");
        as_function(main_func).pureFunction = true;
    }
}
}
