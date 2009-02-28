// Copyright 2008 Andrew Fischer

#include "circa.h"

#include <algorithm>

namespace circa {
namespace min_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::min(as_float(caller->input(0)), as_float(caller->input(1)));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function min(float,float) -> float");
        as_function(main_func).pureFunction = true;
    }
}
} // namespace circa
