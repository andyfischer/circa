// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace add_function {

    void evaluate(Term* caller)
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
            specialize_type(output, FLOAT_TYPE);
            alloc_value(output);
            as_float(output) = to_float(outputTarget) + balanced_delta;

            //std::cout << "for " << format_global_id(output) << ", weight = "
            //    << get_feedback_weight(output) << ", delta = " << delta << std::endl;
        }
    }

    void setup(Branch& kernel)
    {
        ADD_FUNC = import_function(kernel, evaluate, "add(float...) : float");
        as_function(ADD_FUNC).pureFunction = true;
        as_function(ADD_FUNC).feedbackFunc = 
            import_function(kernel, feedback_evaluate, "add_feedback(any, float) : Branch");
    }
}
}
