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
        TaggedValue* value = INPUT(1);
        copy(INPUT(0), OUTPUT);
        TaggedValue* container = OUTPUT;

        if (!is_dict(container))
            set_dict(container);

        copy(value, dict_insert(container, CALLER->stringProp("field").c_str()));
    }
    void setup(Branch* kernel)
    {
        UNPACK_STATE_FUNC = import_function(kernel, unpack_state, "unpack_state(any container) -> any");
        PACK_STATE_FUNC = import_function(kernel, pack_state, "pack_state(any container, any value) -> any");
    }
}
}
