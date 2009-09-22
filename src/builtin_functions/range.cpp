// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace range_function {

    void evaluate(Term* caller)
    {
        int max = as_int(caller->input(0));
        
        Branch& branch = as_branch(caller);

        resize_list(branch, max, INT_TYPE);
        
        for (int i=0; i < max; i++)
            as_int(branch[i]) = i;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "range(int n) : List; 'Return a list of integers from 0 to n' end");
    }
}
} // namespace circa
