// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace add_function {

    void evaluate_i(Term* caller)
    {
        int result = 0;
        for (int i=0; i < caller->numInputs(); i++)
            result += as_int(caller->input(i));
        as_int(caller) = result;
    }

    void evaluate_f(Term* caller)
    {
        float result = 0.0;
        for (int i=0; i < caller->numInputs(); i++)
            result += to_float(caller->input(i));
        as_float(caller) = result;
    }

    void feedback_evaluate(Term* caller)
    {
        Term* target = caller->input(0);
        float desired = to_float(caller->input(1));

        float delta = desired - to_float(target);

        Branch& outputList = as_branch(caller);
        for (int i=0; i < outputList.length(); i++) {
            Term* output = outputList[i];
            Term* outputTarget = target->input(i);
            float balanced_delta = delta * get_feedback_weight(output);
            as_float(output) = to_float(outputTarget) + balanced_delta;
        }
    }

    void setup(Branch& kernel)
    {
        ADD_FUNC = create_overloaded_function(kernel, "add");

        Term* add_i = import_function_overload(ADD_FUNC, evaluate_i, "add_i(int...) : int");

        Term* add_f = import_function_overload(ADD_FUNC, evaluate_f, "add_f(float...) : float");

        function_t::get_feedback_func(add_f) =
            import_function(kernel, feedback_evaluate, "add_feedback(any, float) : Branch");

        kernel.bindName(add_f, "add_f");
        kernel.bindName(add_i, "add_i");
    }
}
}
