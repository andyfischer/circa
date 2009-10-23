// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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
        import_function(kernel, evaluate, "changed(state any, any input) :: bool; 'Stateful function, returns whether the given input has changed since the last call.' end");
    }
}
}
