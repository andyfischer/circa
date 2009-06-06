// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace vectorized_function {

    void evaluate(Term* caller)
    {
        Term* func = as_function(caller->function).parameters[0]->asRef();

        Branch& left = as_branch(caller->input(0));
        Branch& right = as_branch(caller->input(1));

        Branch& output = as_branch(caller);

        if (output.length() == 0) {
            for (int i=0; i < left.length(); i++)
                apply(&output, func, RefList(left[i], right[i]));
        }

        evaluate_branch(output);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "vectorized(List,List) : List");
    }
}
} // namespace circa
