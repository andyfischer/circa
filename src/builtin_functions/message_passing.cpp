// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace message_passing_function {

    static Term* INBOX_FUNC = NULL;

    void evaluate_inbox(EvalContext*, Term* caller)
    {
        copy(caller->input(0), caller);
        as_branch(caller->input(0)).clear();
    }

    void evaluate_send(EvalContext*, Term* caller)
    {
        Term* inbox = caller->input(0);
        Term* value = caller->input(1);

        assert(inbox->function == INBOX_FUNC);

        create_duplicate(as_branch(get_hidden_state_for_call(inbox)), value);
    }

    void setup(Branch& kernel)
    {
        INBOX_FUNC =
            import_function(kernel, evaluate_inbox, "def inbox(state List) -> List");
        import_function(kernel, evaluate_send, "def send(List inbox, any)");
    }
}
}
