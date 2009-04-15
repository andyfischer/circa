// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace range_function {

    void evaluate(Term* caller)
    {
        unsigned int max = as_int(caller->input(0));
        
        Branch& branch = as_branch(caller);
        
        for (unsigned int i=0; i < max; i++) {
            Term* v = create_value(&branch, INT_TYPE);
            as_int(v) = i;
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "range(int) -> List");
        as_function(main_func).pureFunction = true;
    }
}
} // namespace circa
