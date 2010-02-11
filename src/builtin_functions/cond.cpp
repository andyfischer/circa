// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace cond_function {

    void evaluate(EvalContext*, Term* caller)
    {
        int index = bool_input(caller,0) ? 1 : 2;
        Term *result = caller->inputs[index];
        assign_overwriting_type(result, caller);
    }

    Term* specializeType(Term* caller)
    {
        RefList choices(caller->input(1)->type, caller->input(2)->type);
        return find_common_type(choices);
    }

    void feedback_evaluate(EvalContext*, Term* caller)
    {
        Term* target = caller->input(0);
        Term* desired = caller->input(1);
        Branch& output = as_branch(caller);

        // cond(condition, pos, neg)
        //
        // For condition, don't try to send feedback
        assign_value(target->input(0), output[0]);

        // For pos and neg, pass along the feedback that we have received,
        // depending on the value of cond
        bool cond = target->input(0)->asBool();

        int indexUsed = cond ? 1 : 2;

        for (int i=1; i < 2; i++) {
            Term* signal = (i == indexUsed) ? desired : target->input(i);
            assign_value(signal, output[i]);
        }
    }

    void setup(Branch& kernel)
    {
        COND_FUNC = import_function(kernel, evaluate, "cond(bool condition, any pos, any neg) -> any; \"If 'condition' is true, returns 'pos'. Otherwise returns 'neg'.\" end");
        function_t::get_specialize_type(COND_FUNC) = specializeType;
        function_t::set_input_meta(COND_FUNC, 1, true);
        function_t::set_input_meta(COND_FUNC, 2, true);
        function_t::get_feedback_func(COND_FUNC) =
            import_function(kernel, feedback_evaluate, "cond_feedback(any, any) -> Branch");
    }
}
}
