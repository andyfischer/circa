// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace if_function {

    CA_FUNCTION(evaluate)
    {
        if (BOOL_INPUT(0))
            evaluate_branch(CONTEXT, CALLER->nestedContents);
    }

    void setup(Branch& kernel)
    {
        IF_FUNC = import_function(kernel, evaluate, "if(bool) -> any");
    }
}
}
