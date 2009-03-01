// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace set_union_function {

    void evaluate(Term* caller)
    {
        /*
        FIXME
        Set &result = as<Set>(caller);

        for (int inputIndex=0; inputIndex < caller->numInputs(); inputIndex++) {
            Set &input = as<Set>(caller->input(inputIndex));

            std::vector<Term*>::iterator it;

            for (it = input.members.begin(); it != input.members.end(); ++it) {
                result.add(*it);
            }
        }
        */
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function set-union(Set) -> Set");
        as_function(main_func).pureFunction = true;
        as_function(main_func).variableArgs = true;
    }
}
}
