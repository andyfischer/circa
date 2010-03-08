// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace if_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        Branch& contents = caller->asBranch();
        bool cond = bool_input(caller,0);

        if (cond)
            evaluate_branch(cxt, contents);
    }

    void setup(Branch& kernel)
    {
        IF_FUNC = import_function(kernel, evaluate, "if(bool) -> Code");
    }
}
}
