// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace not_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = !bool_input(caller,0);
    }

    void setup(Branch& kernel)
    {
        NOT_FUNC = import_function(kernel, evaluate, "not(bool) : bool");
    }
}
}
