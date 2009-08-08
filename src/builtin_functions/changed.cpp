// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace changed_function {

    void evaluate(Term* caller)
    {
        Term* state = caller->input(0);
        Term* current = caller->input(1);

        if (!equals(state, current)) {
            assign_value(current, state);
            as_bool(caller) = true;
        } else {
            as_bool(caller) = false;
        }
    }

    void setup(Branch& kernel)
    {
        COMMENT_FUNC = import_function(kernel, evaluate, "changed(state any, any) : bool");
    }
}
}
