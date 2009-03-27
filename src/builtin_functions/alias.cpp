// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace alias_function {

    void evaluate(Term* caller)
    {
        caller->value = caller->input(0)->value;
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        ALIAS_FUNC = import_function(kernel, evaluate, "function alias(any) -> any");
        as_function(ALIAS_FUNC).pureFunction = true;
        as_function(ALIAS_FUNC).specializeType = specializeType;
    }
}
}
