// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace div_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_float(caller->input(0)) / as_float(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        DIV_FUNC = import_function(kernel, evaluate, "div(float,float) -> float");
        as_function(DIV_FUNC).pureFunction = true;
    }
}
} // namespace circa
