// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace mult_function {

    void evaluate_f(EvalContext*, Term* caller)
    {
        set_float(caller, float_input(caller,0) * float_input(caller,1));
    }

    void evaluate_i(EvalContext*, Term* caller)
    {
        set_int(caller, int_input(caller,0) * int_input(caller,1));
    }

    void feedback_evaluate(EvalContext*, Term* caller)
    {
        Term* target = caller->input(0);
        float desired = float_input(caller,1);

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
                set_float(output, to_float(outputTarget));
                continue;
            }

            // Otherwise, to solve for x = a * b, tell a that it should be closer to x / b
            set_float(output, to_float(outputTarget) + balanced_delta / divisor);
        }
    }

    void setup(Branch& kernel)
    {
        Term* mult_i = import_function(kernel, evaluate_i, "mult_i(int,int) -> int");
        Term* mult_f = import_function(kernel, evaluate_f, "mult_f(number,number) -> number");

        function_t::get_feedback_func(mult_f) = 
            import_function(kernel, feedback_evaluate, "mult_feedback(any, number) -> Branch");

        MULT_FUNC = create_overloaded_function(kernel, "mult", RefList(mult_i, mult_f));
    }
}
} // namespace circa
