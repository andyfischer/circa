// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace sqrt_function {

    void evaluate(EvalContext*, Term* caller)
    {
        set_float(caller, std::sqrt(float_input(caller,0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "sqrt(number) -> number;"
                "'Square root' end");
    }
}
} // namespace circa
