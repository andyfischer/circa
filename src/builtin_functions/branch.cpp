// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "circa.h"
#include "introspection.h"

namespace circa {
namespace branch_function {

    void evaluate(Term* caller)
    {
        Branch& branch = as<Branch>(caller->state);
        evaluate_branch(branch);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function branch()");
        as_function(main_func).stateType = BRANCH_TYPE;
    }
}
}
