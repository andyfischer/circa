// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace changed_function {

    void evaluate(EvalContext*, Term* caller)
    {
        Term* state = caller->input(0);
        Term* current = caller->input(1);

        bool result;
        if (!equals(state, current)) {
            assign_overwriting_type(current, state);
            result = true;
        } else {
            result = false;
        }
        set_bool(caller, result);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "changed(state any, any input) -> bool; 'Stateful function, returns whether the given input has changed since the last call.' end");
    }
}
}
