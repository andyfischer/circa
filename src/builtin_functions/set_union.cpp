// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace set_union_function {

    void evaluate(Term* caller)
    {
        Branch &result = as_branch(caller);
        result.clear();

        for (int inputIndex=0; inputIndex < caller->numInputs(); inputIndex++) {
            Branch &input = as_branch(caller->input(inputIndex));

            for (int i=0; i < input.numTerms(); i++)
                set_t::add(result, input[i]);
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "set_union(Set) -> Set");
        as_function(main_func).pureFunction = true;
        as_function(main_func).variableArgs = true;
    }
}
}
