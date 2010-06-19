// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace set_union_function {

    void evaluate(EvalContext*, Term* caller)
    {
        List* list = (List*) caller;
        list->clear();

        for (int inputIndex=0; inputIndex < caller->numInputs(); inputIndex++) {
            List* input = (List*) caller->input(inputIndex);
            int numElements = input->numElements();
            for (int i=0; i < numElements; i++)
                set_t::add(list, input->get(i));
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "set_union(Set...) -> Set");
    }
}
}
