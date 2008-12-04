// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace not_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = !as_bool(caller->inputs[0]);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function not(bool) -> bool");
        as_function(main_func).pureFunction = true;
    }
}
}
