// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace list_function {

    void make_list(caStack* stack)
    {
        // Variadic arg handling will already have turned this into a list
        caValue* out = circa_output(stack, 0);
        circa_copy(circa_input(stack, 0), out);
        if (!circa_is_list(out))
            circa_set_list(out, 0);
    }

    void blank_list(caStack* stack)
    {
        caValue* out = circa_output(stack, 0);
        int count = circa_int_input(stack, 0);
        circa_set_list(out, count);
    }

    void setup(Block* kernel)
    {
        FUNCS.list = import_function(kernel, make_list, "list(any vals :multiple) -> List");
        // as_function(FUNCS.list)->specializeType = list_specializeType;

        import_function(kernel, blank_list, "blank_list(int count) -> List");
    }
}
}
