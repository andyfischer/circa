// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace assert_function {

    void evaluate(Term* caller)
    {
        if (!as_bool(caller->input(0))) {
            error_occurred(caller, "Assert failed");
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "assert(bool)");
    }
}
}
