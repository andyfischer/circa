
// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace testing_functions {

    CA_FUNCTION(call_with_state)
    {
        TaggedValue state;
        CONSUME_INPUT(0, &state);
    }
}
}
