// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

#include <algorithm>

namespace circa {
namespace max_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::max(float_input(caller,0), float_input(caller,1));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "max(number,number) -> number; 'Maximum of two numbers' end");
    }
}
} // namespace circa
