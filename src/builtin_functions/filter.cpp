// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace filter_function {

    CA_FUNCTION(evaluate)
    {
        TaggedValue* inputs = INPUT(0);
        TaggedValue* bools = INPUT(1);

        int numInputs = inputs->numElements();
        int numBools = bools->numElements();

        if (numInputs != numBools)
            return error_occurred(CONTEXT, CALLER, "Lists have different lengths");

        // Run through once to count # of trues
        int count = 0;
        for (int i=0; i < numInputs; i++)
            if (bools->getIndex(i)->asBool())
                count++;

        
        List* output = (List*) OUTPUT;
        output->resize(count);
        touch(output);

        int write = 0;
        for (int i=0; i < numInputs; i++) {
            if (bools->getIndex(i)->asBool()) {
                copy((*inputs)[i], output->getIndex(write++));
            }
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "filter(Indexable,Indexable) -> List");
    }
}
} // namespace circa
