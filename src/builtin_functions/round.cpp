// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace round_function {

    void evaluate(Term* caller)
    {
        as_int(caller) = int(to_float(caller->input(0)) + 0.5);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "round(number) : int");
    }
}
}
