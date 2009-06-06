// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace vectorize_vv_function {

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void evaluate(Term* caller)
    {
        Term* func = as_function(caller->function).parameters[0]->asRef();

        Branch& left = as_branch(caller->input(0));
        Branch& right = as_branch(caller->input(1));

        Branch& output = as_branch(caller);

        if (output.length() == 0) {
            for (int i=0; i < left.length(); i++)
                apply(&output, func, RefList(left[i], right[i]));
        }

        evaluate_branch(output);
    }

    void setup(Branch& kernel)
    {
        Term* func = import_function(kernel, evaluate, "vectorize_vv(List,List) : List");
        as_function(func).specializeType = specializeType;
    }
}
} // namespace circa
