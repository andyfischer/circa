// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace vectorize_vs_function {

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void evaluate(EvalContext*, Term* caller)
    {
        Term* func = as_ref(function_t::get_parameters(caller->function));

        Branch& left = as_branch(caller->input(0));
        Term* right = caller->input(1);

        Branch& output = as_branch(caller);

        // Check if our output value has been precreated but not initialized by us.
        if (output.length() > 0 && output[0]->function == VALUE_FUNC)
            output.clear();

        if (output.length() == 0) {
            output.clear();
            for (int i=0; i < left.length(); i++)
                apply(output, func, RefList(left[i], right));
        }

        evaluate_branch(output, caller);
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate, "vectorize_vs(List,any) -> List");
        function_t::get_specialize_type(func) = specializeType;
    }
}
} // namespace circa
