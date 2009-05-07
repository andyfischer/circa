// Copyright 2008 Andrew Fischer

#include "circa.h"

#include <algorithm>

namespace circa {
namespace max_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::max(to_float(caller->input(0)), to_float(caller->input(1)));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "max(float,float) : float");
        as_function(main_func).pureFunction = true;
    }
}
} // namespace circa
