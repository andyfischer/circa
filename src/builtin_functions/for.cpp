// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace for_function {

    void evaluate(Term* caller)
    {
        Branch& series = as_branch(caller->input(0));

        /*
        std::string iterator_name = as_string(inner[iterated_variable_name()]);
        Term* iterator = inner[iterator_name];
        assert(iterator != NULL);

        for (int i=0; i < series.numTerms(); i++) {
            copy_value(series[i], iterator);
            evaluate_branch(inner);
        }
        */
    }

    void setup(Branch& kernel)
    {
        FOR_FUNC = import_function(kernel, evaluate, "function for(List)");
        as_function(FOR_FUNC).pureFunction = true;
        as_function(FOR_FUNC).stateType = BRANCH_TYPE;
    }
}
} // namespace circa
