// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace sub_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_float(caller->input(0)) - as_float(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        SUB_FUNC = import_function(kernel, evaluate, "function sub(float,float) -> float");
        as_function(SUB_FUNC).pureFunction = true;
    }
}
} // namespace circa
