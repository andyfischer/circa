// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace abs_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::abs(to_float(caller->input(0)));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "abs(float) : float");
        as_function(main_func).pureFunction = true;
    }
}
}
