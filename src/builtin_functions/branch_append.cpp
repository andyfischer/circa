// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace branch_append_function {

    void evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);
        create_duplicate(as_branch(caller), caller->input(1));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "branch_append(Branch, any) : Branch");
    }
}
}
