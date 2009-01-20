// Copyright 2008 Paul Hodge

#include "circa.h"

#include "math.h"

namespace circa {
namespace pow_function {

    void evaluate(Term* caller)
    {
        as_int(caller) = (int) pow(as_int(caller->input(0)), as_int(caller->input(1)));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function pow(int,int) -> int");
        as_function(main_func).pureFunction = true;
    }
}
}
