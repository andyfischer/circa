// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace abs_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::abs(to_float(caller->input(0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "abs(float) : float");
    }
}
}
