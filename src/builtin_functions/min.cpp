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
        Term* main_func = import_function(kernel, evaluate, "min(float,float) -> float");
        as_function(main_func).pureFunction = true;
    }
}
} // namespace circa
