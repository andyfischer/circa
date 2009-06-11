// Copyright 2008 Paul Hodge

#include "circa.h"

#include <algorithm>

namespace circa {
namespace min_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::min(to_float(caller->input(0)), to_float(caller->input(1)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "min(float,float) : float");
    }
}
} // namespace circa
