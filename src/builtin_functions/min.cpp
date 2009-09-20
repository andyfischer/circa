// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

#include <algorithm>

namespace circa {
namespace min_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::min(to_float(caller->input(0)), to_float(caller->input(1)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "min(number,number) : number");
    }
}
} // namespace circa
