// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace set_union_function {

    void evaluate(Term* caller)
    {
        Branch &result = as_branch(caller);
        result.clear();

        for (int inputIndex=0; inputIndex < caller->numInputs(); inputIndex++) {
            Branch &input = as_branch(caller->input(inputIndex));

            for (int i=0; i < input.length(); i++)
                set_t::add(result, input[i]);
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "set_union(Set...) -> Set");
    }
}
}
