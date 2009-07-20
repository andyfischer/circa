// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace vectorize_vv_function {

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void evaluate(Term* caller)
    {
        Term* func = function_t::get_parameters(caller->function)[0]->asRef();

        Branch& left = as_branch(caller->input(0));
        Branch& right = as_branch(caller->input(1));

        Branch& output = as_branch(caller);

        // Check if our output value has been precreated but not initialized by us.
        if (output.length() > 0 && output[0]->function == VALUE_FUNC)
            output.clear();

        if (output.length() == 0) {
            output.clear();
            for (int i=0; i < left.length(); i++)
                apply(output, func, RefList(left[i], right[i]));
        }

        evaluate_branch(output);
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate, "vectorize_vv(List,List) : List");
        function_t::get_specialize_type(func) = specializeType;
    }
}
} // namespace circa
