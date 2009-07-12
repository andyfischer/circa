// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace if_function {

    void evaluate(Term* caller)
    {
        Branch& contents = caller->asBranch();
        bool cond = as_bool(caller->input(0));

        if (cond)
            evaluate_branch(contents, caller);
    }

    void setup(Branch& kernel)
    {
        IF_FUNC = import_function(kernel, evaluate, "if(bool) : Branch");
    }
}
}
