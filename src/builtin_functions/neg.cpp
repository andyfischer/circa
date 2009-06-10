// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace neg_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = -to_float(caller->input(0));
    }

    void setup(Branch& kernel)
    {
        NEG_FUNC = import_function(kernel, evaluate, "neg(float):float");
        as_function(NEG_FUNC).pureFunction = true;
    }
}
}
