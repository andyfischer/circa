// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace sub_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = to_float(caller->input(0)) - to_float(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        SUB_FUNC = import_function(kernel, evaluate, "sub(float,float) -> float");
        as_function(SUB_FUNC).pureFunction = true;
    }
}
} // namespace circa
