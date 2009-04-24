// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace not_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = !as_bool(caller->input(0));
    }

    void setup(Branch& kernel)
    {
        NOT_FUNC = import_function(kernel, evaluate, "not(bool) -> bool");
        as_function(NOT_FUNC).pureFunction = true;
    }
}
}
