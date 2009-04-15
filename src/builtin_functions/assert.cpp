// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace assert_function {

    void evaluate(Term* caller)
    {
        if (!as_bool(caller->input(0))) {
            error_occured(caller, "Assert failed");
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "assert(bool)");
        as_function(main_func).pureFunction = false;
    }
}
}
