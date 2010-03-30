// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace inspect_function {

    void get_state(EvalContext*, Term* caller)
    {
        Term* input = caller->input(0);
        Term* hidden_state = get_hidden_state_for_call(input);

        if (hidden_state == NULL)
            change_type(caller, VOID_TYPE);
        else {
            change_type(caller, hidden_state->type);
            copy(hidden_state, caller);
        }
    }

    void get_raw(EvalContext*, Term* caller)
    {
        Term* input = caller->input(0);
        set_str(caller, get_branch_raw(as_branch(input)));
    }

    void setup(Branch& kernel)
    {
        Branch& inspect_ns = create_namespace(kernel, "inspect");
        import_function(inspect_ns, get_state, "get_state(any)->any");
        import_function(inspect_ns, get_raw, "get_raw(Branch)->string");
    }
}
}
