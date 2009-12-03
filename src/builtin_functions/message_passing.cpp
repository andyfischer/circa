// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved

#include <circa.h>

namespace circa {
namespace message_passing_function {

    static Branch* messageStore = NULL;

    void evaluate_send(Term* caller)
    {
        std::string& channel = caller->input(0)->asString();
        Branch& store = *messageStore;

        // check if we need to add a queue for this channel
        if (!store.contains(channel)) {
            create_list(store, channel);
        }

        Branch& queue = store[channel]->asBranch();
        duplicate_value(queue, caller->input(1));
    }

    void evaluate_receive(Term* caller)
    {
        std::string& channel = caller->input(0)->asString();
        Branch& store = *messageStore;
        Branch& output = caller->asBranch();

        output.clear();

        if (!store.contains(channel))
            return;

        assign_value(store[channel], caller);
        store[channel]->asBranch().clear();
    }

    void setup(Branch& kernel)
    {
        messageStore = &create_branch(kernel, "#message_store");

        import_function(kernel, evaluate_send, "def send(string channel, any)");
        import_function(kernel, evaluate_receive, "def receive(string channel)::List");
    }
}
}
