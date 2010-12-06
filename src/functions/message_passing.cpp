// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace message_passing_function {

    static Term* INBOX_FUNC = NULL;

    CA_FUNCTION(evaluate_inbox)
    {
        List* input = List::lazyCast(STATE_INPUT);
        copy(input, OUTPUT);
        input->resize(0);
    }

    CA_FUNCTION(evaluate_send)
    {
        Term* inbox = INPUT_TERM(0);
        Term* input = INPUT_TERM(1);

        ca_assert(inbox->function == INBOX_FUNC);

        List* inboxState = List::lazyCast(get_state_input(CONTEXT, inbox));
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
