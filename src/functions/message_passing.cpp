// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace message_passing_function {

    List* get_message_queue(EvalContext* context, const char* name)
    {
        Dict* hub = &context->messages;
        return List::lazyCast(hub->insert(name));
    }

    CA_FUNCTION(evaluate_send)
    {
        caValue* input = INPUT(1);

        List* inboxState = get_message_queue(CONTEXT, STRING_INPUT(0));

        copy(input, inboxState->append());
    }

    CA_FUNCTION(evaluate_receive)
    {
        List* input = get_message_queue(CONTEXT, STRING_INPUT(0));
        copy(input, OUTPUT);
        input->resize(0);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate_send, "def send(String inbox_name, any)");
        import_function(kernel, evaluate_receive, "def receive(String inbox_name) -> List");
    }
}
}
