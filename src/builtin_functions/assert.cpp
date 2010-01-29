// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace assert_function {

    void evaluate(EvalContext*, Term* caller)
    {
        if (!bool_input(caller,0))
            error_occurred(caller, "Assert failed");
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "assert(bool condition);"
            "'Raises a runtime error if condition is false' end");
    }
}
}
