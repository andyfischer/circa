// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace message_passing_function {

    static Term* INBOX_FUNC = NULL;

    List* get_message_queue(EvalContext* context, Term* term)
    {
        Dict* hub = &context->messages;
        return List::lazyCast(hub->insert(get_unique_name(term)));
    }

    List* get_message_queue(EvalContext* context, const char* name)
    {
        Dict* hub = &context->messages;
        return List::lazyCast(hub->insert(name));
    }

    CA_FUNCTION(evaluate_inbox)
    {
        List* input = get_message_queue(CONTEXT, CALLER);
        copy(input, OUTPUT);
        input->resize(0);
    }

    CA_FUNCTION(evaluate_send)
    {
        Term* inbox = INPUT_TERM(0);
        TaggedValue* input = INPUT(1);

        List* inboxState = NULL;
        if (inbox->function == INBOX_FUNC)
            inboxState = get_message_queue(CONTEXT, inbox);
        else
            inboxState = get_message_queue(CONTEXT, as_cstring(inbox));

        copy(input, inboxState->append());
    }

    CA_FUNCTION(evaluate_receive)
    {
        List* input = get_message_queue(CONTEXT, STRING_INPUT(0));
        copy(input, OUTPUT);
        input->resize(0);
    }

    void setup(Branch& kernel)
    {
        INBOX_FUNC =
            import_function(kernel, evaluate_inbox, "def inbox() -> List");
        import_function(kernel, evaluate_send, "def send(any inbox, any)");
        import_function(kernel, evaluate_receive, "def receive(string inbox_name) -> List");
    }
}
}
