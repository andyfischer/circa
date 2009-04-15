// Copyright 2008 Paul Hodge

#include "circa.h"

#include <cmath>

namespace circa {
namespace pow_function {

    void evaluate(Term* caller)
    {
        as_int(caller) = (int) std::pow((float) as_int(caller->input(0)), as_int(caller->input(1)));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "pow(int,int) -> int");
        as_function(main_func).pureFunction = true;
    }
}
}
