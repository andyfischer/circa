// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace implicit_state_function {
    
    // Unpack a value from a list. The index is given as a static property. This call
    // is used inside if-blocks.
    void unpack_state_from_list(caStack* stack)
    {
        caValue* container = circa_input(stack, 0);
        int index = circa_caller_term(stack)->intProp("index", 0);
        if (!is_list(container) || index >= list_length(container))
            set_null(circa_output(stack, 0));
        else
            copy(list_get(container, index), circa_output(stack, 0));
    }

    // Pack a value to a list. The index is given as a static property. This call
    // is used inside if-blocks.
    void pack_state_to_list(caStack* stack)
    {
        caValue* container = circa_output(stack, 0);
        copy(circa_input(stack, 0), container);
        caValue* value = circa_input(stack, 1);

        int index = circa_caller_term(stack)->intProp("index", 0);

        list_touch(container);

        if (!is_list(container))
            set_list(container, index+1);

        // Set all other elements to null
        list_resize(container, 0);
        list_resize(container, index+1);

        copy(value, list_get(container, index));
    }

    // Unpack a state value from a list. This call is used in for-loops.
    void unpack_state_list_n(caStack* stack)
    {
        caValue* container = circa_input(stack, 0);
        int index = circa_int_input(stack, 1);
        if (!is_list(container) || index >= list_length(container))
            set_null(circa_output(stack, 0));
        else
            copy(list_get(container, index), circa_output(stack, 0));
    }

    // Pack a state value to a list. This call is used in for-loops.
    void pack_state_list_n(caStack* stack)
    {
        caValue* container = circa_output(stack, 0);
        copy(circa_input(stack, 0), container);
        caValue* value = circa_input(stack, 1);
        int index = circa_int_input(stack, 2);
        list_touch(container);

        if (!is_list(container))
            set_list(container, index+1);
        if (list_length(container) <= index)
            list_resize(container, index+1);

        copy(value, list_get(container, index));
    }

    void setup(Block* kernel)
    {
#if 0
        FUNCS.unpack_state = import_function(kernel, (EvaluateFunc) unpack_state,
            "unpack_state(any container, any identifier :meta) -> any");
#endif
        FUNCS.pack_state = import_function(kernel, (EvaluateFunc) pack_state,
            "pack_state(any val :optional :multiple) -> any");

        FUNCS.unpack_state_from_list =
            import_function(kernel, unpack_state_from_list, "unpack_state_from_list(any container) -> any");
        FUNCS.pack_state_to_list =
            import_function(kernel, pack_state_to_list,
                "pack_state_to_list(any container, any value :optional) -> any");

        FUNCS.unpack_state_list_n =
            import_function(kernel, unpack_state_list_n,
            "unpack_state_list_n(any container, int index) -> any");
        FUNCS.pack_state_list_n =
            import_function(kernel, pack_state_list_n,
                "pack_state_list_n(any container, any value :optional, int index) -> any");
    }
}
}
