// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace vectorize_vv_function {

    Term* specializeType(Term* caller)
    {
        if (is_branch(caller->input(0)))
            return caller->input(0)->type;

        return LIST_TYPE;
    }

    void evaluate(Term* caller)
    {
        Term* func = function_t::get_parameters(caller->function)[0]->asRef();

        Branch& left = as_branch(caller->input(0));
        Branch& right = as_branch(caller->input(1));

        if (left.length() != right.length()) {
            std::stringstream msg;
            msg << "Input lists have different lengths (left has " << left.length();
            msg << ", right has " << right.length() << ")";
            error_occurred(caller, msg.str());
            return;
        }

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
        Term* func = import_function(kernel, evaluate, "vectorize_vv(List,List) :: List");
        function_t::get_specialize_type(func) = specializeType;
    }
}
} // namespace circa
