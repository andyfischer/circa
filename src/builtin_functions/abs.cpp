// Copyright (c) 2007-2009 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace abs_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::abs(float_input(caller,0));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "abs(number n) :: number;"
                "'Absolute value' end");
    }
}
}
