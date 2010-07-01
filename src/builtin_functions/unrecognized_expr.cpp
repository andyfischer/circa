// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace unrecognized_expr_function {

    CA_FUNCTION(evaluate)
    {
    }

    void setup(Branch& kernel)
    {
        UNRECOGNIZED_EXPRESSION_FUNC = import_function(kernel, evaluate, "unrecognized_expr()");
    }
}
}
