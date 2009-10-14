// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <circa.h>

namespace circa {
namespace any_true_function {

    void evaluate(Term* caller)
    {
        Branch& input = as_branch(caller->input(0));

        bool result = false;
        for (int i=0; i < input.length(); i++)
            if (input[i]->asBool()) {
                result = true;
                break;
            }

        as_bool(caller) = result;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "any_true(List l) : bool;"
                "'Return whether all of the items in l are true' end");
    }
}
} // namespace circa
