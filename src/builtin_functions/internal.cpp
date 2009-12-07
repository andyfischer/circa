// Copyright (c) 2007-2009 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace internal_function {

    void setup(Branch& kernel)
    {
        LEXPR_FUNC = import_function(kernel, empty_evaluate_function,
            "lexpr(any, string...) :: any");

        LEXPR_FUNC->boolProp("docs:hidden") = true;
    }
}
}
