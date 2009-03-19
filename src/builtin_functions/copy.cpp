// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace copy_function {

    void evaluate(Term* caller)
    {
        // remove this when type parametrezation is in
        if (caller->input(0)->type != caller->type)
            specialize_type(caller, caller->input(0)->type);

        copy_value(caller->input(0), caller);
    }

    void setup(Branch& kernel)
    {
        COPY_FUNC = import_function(kernel, evaluate, "function copy(any) -> any");
        as_function(COPY_FUNC).pureFunction = true;
    }
}
}
