// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace abs_function {

    void evaluate(EvalContext*, Term* caller)
    {
        set_float(caller, std::abs(float_input(caller,0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "abs(number n) -> number;"
                "'Absolute value' end");
    }
}
}
