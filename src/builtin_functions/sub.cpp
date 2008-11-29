// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace sub_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_float(caller->inputs[0]) - as_float(caller->inputs[1]);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function sub(float,float) -> float");
        as_function(main_func).pureFunction = true;
    }
}
} // namespace circa
