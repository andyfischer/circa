// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace if_function {

    CA_FUNCTION(evaluate)
    {
        Branch& contents = CALLER->asBranch();
        bool cond = BOOL_INPUT(0);

        if (cond)
            evaluate_branch(CONTEXT, contents);
    }

    void setup(Branch& kernel)
    {
        IF_FUNC = import_function(kernel, evaluate, "if(bool) -> Code");
    }
}
}
