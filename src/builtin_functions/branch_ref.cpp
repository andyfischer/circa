// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace branch_ref_function {

    void evaluate(Term* caller)
    {
        Branch& refObject = as_branch(caller);
        refObject[0]->asRef() = caller->input(0);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "def branch_ref(Branch branch +ignore_error) -> BranchRef");
    }
}
}
