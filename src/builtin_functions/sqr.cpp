// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace sqr_function {

    void evaluate(Term* caller)
    {
        float in = float_input(caller,0);
        as_float(caller) = in * in;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "sqr(number) -> number; 'Square function' end");
    }
}
} // namespace circa
