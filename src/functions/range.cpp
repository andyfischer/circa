// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdlib>
#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace range_function {

    void evaluate(caStack* stack)
    {
        int start = circa_int_input(stack, 0);
        int max = circa_int_input(stack, 1);

        int count = abs(max - start);
        caValue* output = circa_output(stack, 0);
        set_list(output, count);

        int val = start;
        int increment = start < max ? 1 : -1;
        for (int i=0; i < count; i++) {
            set_int(list_get(output, i), val);
            val += increment;
        }
    }

    Type* specializeType(Term* term)
    {
        Type* type = create_typed_unsized_list_type(TYPES.int_type);
        type_start_at_zero_refs(type);
        return type;
    }

    void setup(Block* kernel)
    {
        FUNCS.range = import_function(kernel, evaluate,
                "range(int start, int max) -> List;"
                "'Return a list of integers from start to max-1'");
        block_set_specialize_type_func(function_contents(FUNCS.range), specializeType);
    }
}
} // namespace circa
