// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace message_passing_function {

    static Term* INBOX_FUNC = NULL;

    void evaluate_inbox(EvalContext*, Term* caller)
    {
        copy(caller->input(0), caller);
#ifdef NEWLIST
        ((List*) caller->input(0))->resize(0);
#else
        as_branch(caller->input(0)).clear();
#endif
    }

    void evaluate_send(EvalContext*, Term* caller)
    {
        Term* inbox = caller->input(0);
        Term* input = caller->input(1);

        assert(inbox->function == INBOX_FUNC);

#ifdef NEWLIST
        List* inboxState = (List*) get_hidden_state_for_call(inbox);
        copy(input, inboxState->append());
#else
        create_duplicate(as_branch(get_hidden_state_for_call(inbox)), input);
#endif
    }

    void setup(Branch& kernel)
    {
        INBOX_FUNC =
            import_function(kernel, evaluate_inbox, "def inbox(state List) -> List");
        import_function(kernel, evaluate_send, "def send(List inbox, any)");
    }
}
}
