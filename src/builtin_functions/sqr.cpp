// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace sqr_function {

    void evaluate(Term* caller)
    {
        float in = float_input(caller,0);
        set_value_float(caller, in * in);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "sqr(number) -> number; 'Square function' end");
    }
}
} // namespace circa
