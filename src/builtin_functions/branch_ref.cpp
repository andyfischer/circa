// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace branch_ref_function {

    void evaluate(EvalContext*, Term* caller)
    {
        branch_ref_t::set_from_ref(caller, caller->input(0));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "def branch_ref(Branch branch +ignore_error) -> BranchRef");
    }
}
}
