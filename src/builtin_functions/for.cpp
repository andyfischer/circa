// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace for_function {

    std::string iterated_variable_name()
    {
        return get_name_for_attribute("iter");
    }

    void evaluate(Term* caller)
    {
        Branch& inner = as_branch(caller);
        Branch& series = as_branch(caller->input(0));

        std::string iterator_name = as_string(inner[iterated_variable_name()]);
        Term* iterator = inner[iterator_name];
        assert(iterator != NULL);

        for (int i=0; i < series.numTerms(); i++) {
            copy_value(series[i], iterator);
            evaluate_branch(inner);
        }
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "function for(Branch)");
        as_function(main_func).pureFunction = true;
        as_function(main_func).stateType = BRANCH_TYPE;
    }
}
} // namespace circa
