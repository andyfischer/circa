// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace changed_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(changed, "changed(state any, any input) -> bool;"
        "'Stateful function, returns whether the given input has changed since the "
        "last call.'")
    {
        TaggedValue* state = STATE_INPUT;
        TaggedValue* current = INPUT(1);

        bool result;
        if (!equals(state, current)) {
            copy(current, state);
            result = true;
        } else {
            result = false;
        }
        set_bool(OUTPUT, result);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
