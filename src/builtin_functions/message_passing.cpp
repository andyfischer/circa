// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace message_passing_function {

    static Term* INBOX_FUNC = NULL;

    void evaluate_inbox(EvalContext*, Term* caller)
    {
        copy(caller->input(0), caller);
        ((List*) caller->input(0))->resize(0);
    }

    void evaluate_send(EvalContext*, Term* caller)
    {
        Term* inbox = caller->input(0);
        Term* input = caller->input(1);

        assert(inbox->function == INBOX_FUNC);

        List* inboxState = (List*) get_hidden_state_for_call(inbox);
        copy(input, inboxState->append());
    }

    void setup(Branch& kernel)
    {
        INBOX_FUNC =
            import_function(kernel, evaluate_inbox, "def inbox(state List) -> List");
        import_function(kernel, evaluate_send, "def send(List inbox, any)");
    }
}
}
