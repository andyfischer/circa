// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

// alias() is used to describe a value that should always be the same as the input
// value, and no copy is needed. In the future we'll perform optimizations based
// on this assumption.

namespace circa {
namespace alias_function {


    void evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        ALIAS_FUNC = import_function(kernel, evaluate, "alias(any) : any");
        function_t::get_specialize_type(ALIAS_FUNC) = specializeType;
    }
}
}
