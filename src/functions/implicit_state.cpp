// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace implicit_state_function {

    CA_FUNCTION(unpack_state)
    {
        caValue* container = INPUT(0);
        Term* identifyingTerm = INPUT_TERM(1);

        if (!is_dict(container)) {
            set_null(OUTPUT);
            return;
        }

        const char* fieldName = unique_name(identifyingTerm);
        copy(dict_insert(container, fieldName), OUTPUT);
    }

    CA_FUNCTION(pack_state)
    {
        copy(INPUT(0), OUTPUT);
        caValue* container = OUTPUT;
        caValue* value = INPUT(1);
        Term* identifyingTerm = INPUT_TERM(2);

        if (!is_dict(container))
            set_dict(container);

        const char* fieldName = unique_name(identifyingTerm);
        copy(value, dict_insert(container, fieldName));
    }

    // Used in if_block. The index is built in as a property.
    CA_FUNCTION(unpack_state_list)
    {
        caValue* container = INPUT(0);
        int index = CALLER->intProp("index");
        if (!is_list(container) || index >= list_length(container))
            set_null(OUTPUT);
        else
            copy(list_get_index(container, index), OUTPUT);
    }

    // Used in if_block. The index is built in as a property.
    CA_FUNCTION(pack_state_to_list)
    {
        copy(INPUT(0), OUTPUT);
        caValue* container = OUTPUT;
        caValue* value = INPUT(1);

        int index = CALLER->intProp("index");

        if (!is_list(container))
            set_list(container, index+1);

        // Set all other elements to null
        list_resize(container, 0);
        list_resize(container, index+1);

        copy(value, list_get_index(container, index));
    }

    // Used in for_loop
    CA_FUNCTION(unpack_state_list_n)
    {
        caValue* container = INPUT(0);
        int index = INT_INPUT(1);
        if (!is_list(container) || index >= list_length(container))
            set_null(OUTPUT);
        else
            copy(list_get_index(container, index), OUTPUT);
    }
    // Used in for_loop
    CA_FUNCTION(pack_state_list_n)
    {
        copy(INPUT(0), OUTPUT);
        caValue* container = OUTPUT;
        caValue* value = INPUT(1);
        int index = INT_INPUT(2);

        if (!is_list(container))
            set_list(container, index+1);
        if (list_length(container) <= index)
            list_resize(container, index+1);

        copy(value, list_get_index(container, index));
    }

    void setup(Branch* kernel)
    {
        FUNCS.unpack_state = import_function(kernel, unpack_state,
            "unpack_state(any container, any identifier :meta) -> any");
        FUNCS.pack_state = import_function(kernel, pack_state,
            "pack_state(any container, any value, any identifier :meta) -> any");

        FUNCS.unpack_state_list =
            import_function(kernel, unpack_state_list, "unpack_state_list(any container) -> any");
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
