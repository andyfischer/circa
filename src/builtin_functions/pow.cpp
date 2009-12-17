// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

#include <cmath>

namespace circa {
namespace pow_function {

    void evaluate(Term* caller)
    {
        as_int(caller) = (int) std::pow((float) int_input(caller,0), int_input(caller,1));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "pow(int i, int x) -> int; 'Returns i to the power of x' end");
    }
}
}
