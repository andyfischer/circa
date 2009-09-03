// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

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
        import_function(kernel, evaluate, "range(int) : List");
    }
}
} // namespace circa
