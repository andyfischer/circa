// Copyright 2008 Paul Hodge

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
    }
}
}
