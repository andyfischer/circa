// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace unrecognized_expr_function {

    void evaluate(Term* caller)
    {
    }

    void setup(Branch& kernel)
    {
        UNRECOGNIZED_EXPRESSION_FUNC = import_function(kernel, evaluate,
                "function unrecognized-expr()");
        as_function(UNRECOGNIZED_EXPRESSION_FUNC).stateType = STRING_TYPE;
        as_function(UNRECOGNIZED_EXPRESSION_FUNC).pureFunction = false;
        as_function(UNRECOGNIZED_EXPRESSION_FUNC).hasSideEffects = true;
    }
}
}
