// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace range_function {

    void evaluate(Term* caller)
    {
        int start = int_input(caller, 0);
        int max = int_input(caller, 1);
        
        Branch& branch = as_branch(caller);

        resize_list(branch, max-start, INT_TYPE);
        
        for (int i=start; i < max; i++)
            as_int(branch[i]) = i;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "range(int start, int max) :: List; 'Return a list of integers from start to max-1' end");
    }
}
} // namespace circa
