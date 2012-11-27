// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace add_function {

    void add_i_evaluate(caStack* stack)
    {
        int sum = circa_int_input(stack, 0) + circa_int_input(stack, 1);
        set_int(circa_output(stack, 0), sum);
    }

    void add_f_evaluate(caStack* stack)
    {
        float sum = circa_float_input(stack, 0) + circa_float_input(stack, 1);
        set_float(circa_output(stack, 0), sum);
    }

#if 0
    void add_dynamic(caStack* stack)
    {
        bool allInts = true;
        for (int i=0; i < NUM_INPUTS; i++) {
            if (!is_int(INPUT(0)))
                allInts = false;
        }

        if (allInts)
            add_i_evaluate(_cxt, _ins, _outs);
        else
            add_f_evaluate(_cxt, _ins, _outs);
    }
#endif

    void add_feedback(caStack* stack)
    {
        #if 0
        OLD_FEEDBACK_IMPL_DISABLED
        Term* target = INPUT_TERM(0);
        float desired = FLOAT_INPUT(1);

        float delta = desired - to_float(target);

        Block* outputList = feedback_output(CALLER);
        for (int i=0; i < outputList.length(); i++) {
            Term* output = outputList[i];
            Term* outputTarget = target->input(i);
            float balanced_delta = delta * get_feedback_weight(output);
            set_float(output, to_float(outputTarget) + balanced_delta);
        }
        #endif
    }

    void setup(Block* kernel)
    {
        FUNCS.add_i = import_function(kernel, add_i_evaluate, "add_i(int a, int b) -> int");
        FUNCS.add_f = import_function(kernel, add_f_evaluate, "add_f(number a, number b) -> number");
    }
} // namespace add_function
} // namespace circa
