// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace set_union_function {

    void evaluate(EvalContext*, Term* caller)
    {
#ifdef NEWLIST
        List* list = (List*) caller;
        list->clear();

        for (int inputIndex=0; inputIndex < caller->numInputs(); inputIndex++) {
            List* input = (List*) caller->input(inputIndex);
            int numElements = input->numElements();
            for (int i=0; i < numElements; i++)
                set_t::add(list, input->get(i));
        }
#else
        Branch &result = as_branch(caller);
        result.clear();

        for (int inputIndex=0; inputIndex < caller->numInputs(); inputIndex++) {
            Branch &input = as_branch(caller->input(inputIndex));

            for (int i=0; i < input.length(); i++)
                set_t::add(result, input[i]);
        }
#endif
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "set_union(Set...) -> Set");
    }
}
}
