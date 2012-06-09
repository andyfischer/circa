// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace implicit_state_function {
    
    // Unpack a value from a list. The index is given as a static property. This call
    // is used inside if-blocks.
    CA_FUNCTION(unpack_state_from_list)
    {
        caValue* container = INPUT(0);
        int index = CALLER->intProp("index", 0);
        if (!is_list(container) || index >= list_length(container))
            set_null(OUTPUT);
        else
            copy(list_get(container, index), OUTPUT);
    }

    // Pack a value to a list. The index is given as a static property. This call
    // is used inside if-blocks.
    CA_FUNCTION(pack_state_to_list)
    {
        copy(INPUT(0), OUTPUT);
        caValue* container = OUTPUT;
        caValue* value = INPUT(1);

        int index = CALLER->intProp("index", 0);

        list_touch(container);

        if (!is_list(container))
            set_list(container, index+1);

        // Set all other elements to null
        list_resize(container, 0);
        list_resize(container, index+1);

        copy(value, list_get(container, index));
    }

    // Unpack a state value from a list. This call is used in for-loops.
    CA_FUNCTION(unpack_state_list_n)
    {
        caValue* container = INPUT(0);
        int index = INT_INPUT(1);
        if (!is_list(container) || index >= list_length(container))
            set_null(OUTPUT);
        else
            copy(list_get(container, index), OUTPUT);
    }

    // Pack a state value to a list. This call is used in for-loops.
    CA_FUNCTION(pack_state_list_n)
    {
        copy(INPUT(0), OUTPUT);
        caValue* container = OUTPUT;
        caValue* value = INPUT(1);
        int index = INT_INPUT(2);
        list_touch(container);

        if (!is_list(container))
            set_list(container, index+1);
        if (list_length(container) <= index)
            list_resize(container, index+1);

        copy(value, list_get(container, index));
    }

    void setup(Branch* kernel)
    {
        FUNCS.unpack_state = import_function(kernel, (EvaluateFunc) unpack_state,
            "unpack_state(any container, any identifier :meta) -> any");
        FUNCS.pack_state = import_function(kernel, (EvaluateFunc) pack_state,
            "pack_state(any :optional :multiple) -> any");

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
