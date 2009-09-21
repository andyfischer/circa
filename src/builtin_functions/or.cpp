// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace or_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = as_bool(caller->input(0)) || as_bool(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "or(bool a, bool b) : bool;"
            "'Return whether either a or b are true' end");
    }
}
} // namespace circa
