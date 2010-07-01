// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace inspect_function {

    CA_FUNCTION(get_state)
    {
        Term* input = INPUT_TERM(0);
        Term* hidden_state = get_hidden_state_for_call(input);

        if (hidden_state == NULL)
            change_type(CALLER, VOID_TYPE);
        else {
            change_type(CALLER, hidden_state->type);
            copy(hidden_state, OUTPUT);
        }
    }

    CA_FUNCTION(get_raw)
    {
        set_str(OUTPUT, get_branch_raw(as_branch(INPUT(0))));
    }

    void setup(Branch& kernel)
    {
        Branch& inspect_ns = create_namespace(kernel, "inspect");
        import_function(inspect_ns, get_state, "get_state(any)->any");
        import_function(inspect_ns, get_raw, "get_raw(Branch)->string");
    }
}
}
