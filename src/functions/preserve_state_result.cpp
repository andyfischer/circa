// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace preserve_state_result_function {
    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(preserve_state_result, "preserve_state_result(any)")
    {
        TaggedValue result;
        copy(INPUT(0), &result);
        // TODO: switch to use consume_input instead of copy
        //consume_input(CALLER, 0, &result);

        // Use 'name' here, not uniqueName.
        const char* name = INPUT_TERM(0)->name.c_str();
        Dict* state = Dict::lazyCast(&CONTEXT->currentScopeState);
        swap(&result, state->insert(name));
        set_null(OUTPUT);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        PRESERVE_STATE_RESULT_FUNC = kernel["preserve_state_result"];
    }
}
}
