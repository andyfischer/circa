// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace branch_mirror_function {

    void evaluate(Term* caller)
    {
        Branch& mirrorObject = as_branch(caller);
        mirrorObject[0]->asRef() = caller->input(0);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "def branch_mirror(Branch branch +ignore_error) -> BranchMirror");
    }
}
}
