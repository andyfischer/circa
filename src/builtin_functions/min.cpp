// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

#include <algorithm>

namespace circa {
namespace min_function {

    void evaluate(Term* caller)
    {
        set_float(caller, std::min(float_input(caller,0), float_input(caller,1)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "min(number,number) -> number; 'Minimum of two numbers.' end");
    }
}
} // namespace circa
