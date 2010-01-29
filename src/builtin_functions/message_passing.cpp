// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace message_passing_function {

    static Branch* messageStore = NULL;

    void evaluate_send(EvalContext*, Term* caller)
    {
        std::string channelName = caller->input(0)->asString();
        Branch& store = *messageStore;

        // check if we need to add a queue for this channelName
        if (!store.contains(channelName)) {
            create_list(store, channelName);
        }

        Branch& queue = store[channelName]->asBranch();
        duplicate_value(queue, caller->input(1));
    }

    void evaluate_receive(EvalContext*, Term* caller)
    {
        std::string channelName = caller->input(0)->asString();
        Branch& store = *messageStore;
        Branch& output = caller->asBranch();

        output.clear();

        if (!store.contains(channelName))
            return;

        assign_value(store[channelName], caller);
        store[channelName]->asBranch().clear();
    }

    void setup(Branch& kernel)
    {
        messageStore = &create_branch(kernel, "#message_store");

        import_function(kernel, evaluate_send, "def send(string channel, any)");
        import_function(kernel, evaluate_receive, "def receive(string channel) -> List");
    }
}
}
