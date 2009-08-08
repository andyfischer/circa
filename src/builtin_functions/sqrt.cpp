// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace sqrt_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::sqrt(caller->input(0)->toFloat());
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "sqrt(float) : float");
    }
}
} // namespace circa
