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
        import_function(kernel, evaluate, "sqrt(float) : float");
    }
}
} // namespace circa
