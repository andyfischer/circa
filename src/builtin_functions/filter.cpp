// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace filter_function {

    void evaluate(Term* caller)
    {
        Branch& inputs = as_branch(caller->input(0));
        Branch& bools = as_branch(caller->input(1));
        Branch& output = as_branch(caller);

        if (inputs.length() != bools.length()) {
            error_occurred(caller, "Lists have different lengths");
            return;
        }

        int write = 0;

        for (int i=0; i < inputs.length(); i++) {
            if (bools[i]->asBool()) {
                if (output.length() <= i) {
                    create_value(output, inputs[i]->type);
                }

                assign_value(inputs[i], output[write]);
                write++;
            }
        }

        // Remove extra elements
        for (int i=write; i < output.length(); i++)
            output.set(i, NULL);

        output.removeNulls();
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "filter(List,List) -> List");
    }
}
} // namespace circa
