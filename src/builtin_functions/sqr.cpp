// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace sqr_function {

    void evaluate(Term* caller)
    {
        float in = to_float(caller->input(0));
        as_float(caller) = in * in;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "sqr(float) : float");
    }
}
} // namespace circa
