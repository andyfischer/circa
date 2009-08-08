// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace map_function {

    void evaluate(Term* caller)
    {
        Term* func = caller->input(0);
        Branch& inputs = as_branch(caller->input(1));
        Branch& output = as_branch(caller);

        // Create term if necessary
        for (int i=output.length(); i < inputs.length(); i++)
            apply(output, func, RefList(inputs[i]));

        // Remove extra terms if necessary
        for (int i=inputs.length(); i < output.length(); i++)
            output[i] = NULL;

        output.removeNulls();

        evaluate_branch(output);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "map(any,List) : List");
    }
}
} // namespace circa
