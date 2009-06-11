// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace range_function {

    void evaluate(Term* caller)
    {
        unsigned int max = as_int(caller->input(0));
        
        Branch& branch = as_branch(caller);
        branch.clear();
        for (unsigned int i=0; i < max; i++) {
            Term* v = create_value(&branch, INT_TYPE);
            as_int(v) = i;
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "range(int) : List");
    }
}
} // namespace circa
