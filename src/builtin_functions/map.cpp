// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace map_function {

    void evaluate(Term* caller)
    {
        Term* func = caller->input(0);
        Branch& inputs = as_branch(caller->input(1));
        Branch& output = as_branch(caller);

        if (output.length() == 0) {
            for (int i=0; i < inputs.length(); i++) {
                apply(&output, func, RefList(inputs[i]));
            }
        }

        evaluate_branch(output);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "map(Function,List) : List");
    }
}
} // namespace circa
