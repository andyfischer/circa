// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace mult_function {

    CA_FUNCTION(evaluate_f)
    {
        float product = circa_float_input(STACK, 0) * circa_float_input(STACK, 1);
        set_float(circa_output(STACK, 0), product);
    }

    CA_FUNCTION(evaluate_i)
    {
        int product = circa_int_input(STACK, 0) * circa_int_input(STACK, 1);
        set_int(circa_output(STACK, 0), product);
    }

    CA_FUNCTION(feedback_evaluate)
    {
#if 0
OLD_FEEDBACK_IMPL_DISABLED
        Term* target = INPUT_TERM(0);
        float desired = FLOAT_INPUT(1);

        float delta = desired - to_float(target);

        // for each input, send a delta divided by the product of all other inputs
        Block* outputList = feedback_output(CALLER);
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
        #endif
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate_i, "mult_i(int a,int b) -> int");
        import_function(kernel, evaluate_f, "mult_f(number a,number b) -> number");
    }
}
} // namespace circa
