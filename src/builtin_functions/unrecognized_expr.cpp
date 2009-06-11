// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace unrecognized_expr_function {

    void evaluate(Term* caller)
    {
    }

    void setup(Branch& kernel)
    {
        UNRECOGNIZED_EXPRESSION_FUNC = import_function(kernel, evaluate, "unrecognized_expr()");
    }
}
}
