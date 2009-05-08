// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace sqrt_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::sqrt(caller->input(0)->toFloat());
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate, "sqrt(float) : float");
        as_function(func).pureFunction = true;
    }
}
} // namespace circa
