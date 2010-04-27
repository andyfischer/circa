// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace filter_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        TaggedValue* inputs = caller->input(0);
        TaggedValue* bools = caller->input(1);
        Branch& output = as_branch(caller);

        int numInputs = inputs->numElements();
        int numBools = bools->numElements();

        if (numInputs != numBools)
            return error_occurred(cxt, caller, "Lists have different lengths");

        int write = 0;

        for (int i=0; i < numInputs; i++) {
            if (as_bool((*bools)[i])) {
                if (output.length() <= i)
                    output.appendNew();

                copy((*inputs)[i], output[write]);
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
        import_function(kernel, evaluate, "filter(Indexable,Indexable) -> List");
    }
}
} // namespace circa
