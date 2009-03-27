// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace copy_function {

    void evaluate(Term* caller)
    {
        copy_value(caller->input(0), caller);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        COPY_FUNC = import_function(kernel, evaluate, "function copy(any) -> any");
        as_function(COPY_FUNC).pureFunction = true;
        as_function(COPY_FUNC).specializeType = specializeType;
    }
}
}
