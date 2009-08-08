// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace div_i_function {

    void evaluate(Term* caller)
    {
        as_int(caller) = as_int(caller->input(0)) / as_int(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "div_i(int,int) : int");
    }
}
} // namespace circa
