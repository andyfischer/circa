// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace get_field_function {

    void evaluate(caStack* stack)
    {
        caValue* head = circa_input(stack, 0);

        Value error;
        caValue* value = get_field(head, circa_input(stack, 1), &error);

        if (!is_null(&error)) {
            circa_output_error(stack, as_cstring(&error));
            return;
        }

        ca_assert(value != NULL);

        copy(value, circa_output(stack, 0));
    }

    Type* specializeType(Term* caller)
    {
        Type* head = caller->input(0)->type;

        for (int nameIndex=1; nameIndex < caller->numInputs(); nameIndex++) {

            // Abort if input type is not correct
            if (!is_string(term_value(caller->input(1))))
                return TYPES.any;

            if (!is_list_based_type(head))
                return TYPES.any;

            std::string const& name = as_string(term_value(caller->input(1)));

            int fieldIndex = list_find_field_index_by_name(head, name.c_str());

            if (fieldIndex == -1)
                return TYPES.any;

            head = as_type(get_index(list_get_type_list_from_type(head),fieldIndex));
        }

        return head;
    }

    void setup(Block* kernel)
    {
        FUNCS.get_field = import_function(kernel, evaluate,
                "get_field(any obj, String key) -> any");
        block_set_specialize_type_func(function_contents(FUNCS.get_field), specializeType);
    }
}
}
