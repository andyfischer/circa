// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace range_function {

    CA_FUNCTION(evaluate)
    {
        int start = INT_INPUT(0);
        int max = INT_INPUT(1);

        int count = abs(max-start);
        set_list(OUTPUT);
        List* list = List::checkCast(OUTPUT);
        list->resize(count);

        int val = start;
        int increment = start < max ? 1 : -1;
        for (int i=0; i < count; i++) {
            set_int(list->get(i), val);
            val += increment;
        }
    }

    Type* specializeType(Term* term)
    {
        // TODO: Should reuse existing type object
        return create_typed_unsized_list_type(&INT_T);
    }

    void setup(Branch* kernel)
    {
        RANGE_FUNC = import_function(kernel, evaluate,
                "range(int start, int max) -> List;"
                "'Return a list of integers from start to max-1'");
        as_function(RANGE_FUNC)->specializeType = specializeType;
    }
}
} // namespace circa
