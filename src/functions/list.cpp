// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace list_function {

#if 0 // Generic types are incomplete and disabled
    Type* specializeType(Term* term)
    {
        List inputTypes;
        for (int i=0; i < term->numInputs(); i++)
            set_type(inputTypes.append(), term->input(i)->type);

        return as_type(create_tuple_type(&inputTypes));
    }
#endif

    void make_list(caStack* stack)
    {
        // Variadic arg handling will already have turned this into a list
        caValue* out = circa_output(stack, 0);
        circa_copy(circa_input(stack, 0), out);
        if (!circa_is_list(out))
            circa_set_list(out, 0);
    }

    void repeat(caStack* stack)
    {
        caValue* source = circa_input(stack, 0);
        int repeatCount = circa_int_input(stack, 1);

        caValue* out = circa_output(stack, 0);
        circa_set_list(out, repeatCount);

        for (int i=0; i < repeatCount; i++)
            copy(source, circa_index(out, i));
    }

    void blank_list(caStack* stack)
    {
        caValue* out = circa_output(stack, 0);
        int count = circa_int_input(stack, 0);
        circa_set_list(out, count);
    }

    void setup(Branch* kernel)
    {
        FUNCS.list = import_function(kernel, make_list, "list(any :multiple) -> List");
        import_function(kernel, repeat,  "repeat(any, int) -> List");
        import_function(kernel, blank_list, "blank_list(int) -> List");
    }
}
}
