// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace not_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = !as_bool(caller->input(0));
    }

    void setup(Branch& kernel)
    {
        NOT_FUNC = import_function(kernel, evaluate, "not(bool) : bool");
    }
}
}
