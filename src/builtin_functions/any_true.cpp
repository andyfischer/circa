// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace any_true_function {

    void evaluate(EvalContext*, Term* caller)
    {
        TaggedValue* input = caller->input(0);

        int numElements = input->numElements();

        bool result = false;
        for (int i=0; i < numElements; i++)
            if (as_bool((*input)[i])) {
                result = true;
                break;
            }

        set_bool(caller, result);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "any_true(List l) -> bool;"
                "'Return whether any of the items in l are true' end");
    }
}
} // namespace circa
