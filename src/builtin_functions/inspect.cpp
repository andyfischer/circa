// Copyright (c) 2007-2009 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace inspect_function {

    void get_state(Term* caller)
    {
        Term* input = caller->input(0);
        Term* hidden_state = get_hidden_state_for_call(input);

        if (hidden_state == NULL)
            change_type(caller, VOID_TYPE);
        else {
            change_type(caller, hidden_state->type);
            assign_value(hidden_state, caller);
        }
    }

    void get_raw(Term* caller)
    {
        Term* input = caller->input(0);
        as_string(caller) = get_branch_raw(as_branch(input));
    }

    void setup(Branch& kernel)
    {
        Branch& inspect_ns = create_namespace(kernel, "inspect");
        import_function(inspect_ns, get_state, "get_state(any)->any");
        import_function(inspect_ns, get_raw, "get_raw(Branch)->string");
    }
}
}
