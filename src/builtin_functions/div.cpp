// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace div_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_float(caller->inputs[0]) / as_float(caller->inputs[1]);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function div(float,float) -> float");
        as_function(main_func).pureFunction = true;
    }
}
} // namespace circa
