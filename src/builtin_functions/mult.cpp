// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace mult_function {

    void evaluate_f(Term* caller)
    {
        as_float(caller) = to_float(caller->input(0)) * to_float(caller->input(1));
    }

    void evaluate_i(Term* caller)
    {
        as_int(caller) = as_int(caller->input(0)) * as_int(caller->input(1));
    }

    void feedback_evaluate(Term* caller)
    {
        Term* target = caller->input(0);
        float desired = to_float(caller->input(1));

        float delta = desired - to_float(target);

        // for each input, send a delta divided by the product of all other inputs
        Branch& outputList = as_branch(caller);
        for (int i=0; i < outputList.length(); i++) {
            Term* output = outputList[i];
            Term* outputTarget = target->input(i);
            float balanced_delta = delta * get_feedback_weight(output);

            // Compute a product of all other inputs
            float divisor = i == 0 ? to_float(target->input(1)) : to_float(target->input(0));

            // If this product is too close to 0 then give up. We can't solve x = a * 0
            if (fabs(divisor) < 0.0001) {
                as_float(output) = to_float(outputTarget);
                continue;
            }

            //std::cout << "divisor = " << divisor << ", delta = " << balanced_delta << std::endl;

            // Otherwise, to solve for x = a * b, tell a that it should be closer to x / b
            as_float(output) = to_float(outputTarget) + balanced_delta / divisor;
        }
    }

    void setup(Branch& kernel)
    {
        MULT_FUNC = create_overloaded_function(kernel, "mult");

        Term* mult_i = import_function_overload(MULT_FUNC, evaluate_i, "mult_i(int,int) : int");

        Term* mult_f = import_function_overload(MULT_FUNC, evaluate_f, "mult_f(float,float) : float");

        function_t::get_feedback_func(mult_f) = 
            import_function(kernel, feedback_evaluate, "mult_feedback(any, float) : Branch");

        kernel.bindName(mult_i, "mult_i");
        kernel.bindName(mult_f, "mult_f");
    }
}
} // namespace circa
