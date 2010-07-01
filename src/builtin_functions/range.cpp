// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace range_function {

    CA_FUNCTION(evaluate)
    {
        int start = INT_INPUT(0);
        int max = INT_INPUT(1);

        int count = abs(max-start);
        List* list = (List*) OUTPUT;
        list->resize(count);

        int val = start;
        int increment = start < max ? 1 : -1;
        for (int i=0; i < count; i++) {
            make_int(list->get(i), val);
            val += increment;
        }
    }

    void setup(Branch& kernel)
    {
        RANGE_FUNC = import_function(kernel, evaluate,
                "range(int start, int max) -> List; 'Return a list of integers from start to max-1' end");
    }
}
} // namespace circa
