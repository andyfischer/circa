// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "dict.h"

namespace circa {
namespace implicit_state_function {

    CA_FUNCTION(unpack_state)
    {
        TaggedValue* container = INPUT(0);

        if (!is_dict(container))
            set_null(OUTPUT);
        else
            copy(dict_insert(container, CALLER->stringProp("field").c_str()), OUTPUT);
    }

    CA_FUNCTION(pack_state)
    {
        copy(INPUT(0), OUTPUT);
        TaggedValue* container = OUTPUT;
        TaggedValue* value = INPUT(1);

        if (!is_dict(container))
            set_dict(container);

        copy(value, dict_insert(container, CALLER->stringProp("field").c_str()));
    }

    CA_FUNCTION(unpack_state_list)
    {
        TaggedValue* container = INPUT(0);
        int index = CALLER->intProp("index");
        if (!is_list(container) || index >= list_get_length(container))
            set_null(OUTPUT);
        else
            copy(list_get_index(container, index), OUTPUT);
    }

    CA_FUNCTION(pack_state_to_list)
    {
        copy(INPUT(0), OUTPUT);
        TaggedValue* container = OUTPUT;
        TaggedValue* value = INPUT(1);

        int index = CALLER->intProp("index");

        if (!is_list(container))
            set_list(container, index+1);

        // Set all other elements to null
        list_resize(container, 0);
        list_resize(container, index+1);

        copy(value, list_get_index(container, index));
    }

    void setup(Branch* kernel)
    {
        BUILTIN_FUNCS.unpack_state = import_function(kernel, unpack_state, "unpack_state(any container) -> any");
        BUILTIN_FUNCS.pack_state = import_function(kernel, pack_state, "pack_state(any container, any value) -> any");

        BUILTIN_FUNCS.unpack_state_list =
            import_function(kernel, unpack_state_list, "unpack_state_list(any container) -> any");
        BUILTIN_FUNCS.pack_state_to_list =
            import_function(kernel, pack_state_to_list,
                "pack_state_to_list(any container, any value :optional) -> any");
    }
}
}
