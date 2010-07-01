// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace cond_function {

    CA_FUNCTION(cond_evaluate)
    {
        int index = BOOL_INPUT(0) ? 1 : 2;
        copy(INPUT(index), OUTPUT);
    }

    Term* specializeType(Term* caller)
    {
        RefList choices(caller->input(1)->type, caller->input(2)->type);
        return find_common_type(choices);
    }

    CA_FUNCTION(feedback)
    {
        Term* target = INPUT_TERM(0);
        Term* desired = INPUT_TERM(1);
        Branch& output = as_branch(OUTPUT);

        // cond(condition, pos, neg)
        //
        // For condition, don't try to send feedback
        copy(target->input(0), output[0]);

        // For pos and neg, pass along the feedback that we have received,
        // depending on the value of cond
        bool cond = target->input(0)->asBool();

        int indexUsed = cond ? 1 : 2;

        for (int i=1; i <= 2; i++) {
            Term* signal = (i == indexUsed) ? desired : target->input(i);
            copy(signal, output[i]);
        }
    }

    void setup(Branch& kernel)
    {
        COND_FUNC = import_function(kernel, cond_evaluate, "cond(bool condition, any pos, any neg) -> any; \"If 'condition' is true, returns 'pos'. Otherwise returns 'neg'.\" end");
        function_t::get_specialize_type(COND_FUNC) = specializeType;
        function_t::set_input_meta(COND_FUNC, 1, true);
        function_t::set_input_meta(COND_FUNC, 2, true);
        function_t::get_feedback_func(COND_FUNC) =
            import_function(kernel, feedback, "cond_feedback(any, any) -> Branch");
    }
}
}
