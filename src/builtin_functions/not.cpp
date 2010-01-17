// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace not_function {

    void evaluate(Term* caller)
    {
        set_bool(caller, !bool_input(caller,0));
    }

    void setup(Branch& kernel)
    {
        NOT_FUNC = import_function(kernel, evaluate, "not(bool) -> bool");
    }
}
}
