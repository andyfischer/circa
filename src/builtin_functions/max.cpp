// Copyright 2008 Paul Hodge

#include "circa.h"

#include <algorithm>

namespace circa {
namespace max_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::max(as_float(caller->input(0)), as_float(caller->input(1)));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function max(float,float) -> float");
        as_function(main_func).pureFunction = true;
    }
}
} // namespace circa
