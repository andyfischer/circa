// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "circa.h"
#include "introspection.h"

namespace circa {
namespace branch_function {

    void evaluate(Term* caller)
    {
        Branch& branch = as_branch(caller);
        evaluate_branch(branch);
    }

    void setup(Branch& kernel)
    {
        BRANCH_FUNC = import_function(kernel, evaluate, "branch() : Branch");
    }
}
}
