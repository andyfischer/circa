// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <circa.h>

namespace circa {
namespace round_function {

    void evaluate(Term* caller)
    {
        as_int(caller) = int(to_float(caller->input(0)) + 0.5);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "round(number) : int; 'Round off the given number to the nearest integer' end");
    }
}
}
