// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace if_expr_function {

    void evaluate(Term* caller)
    {
        int index = as_bool(caller->input(0)) ? 1 : 2;
        Term *result = caller->inputs[index];

        if (!is_value_alloced(result)) {
            std::stringstream out;
            out << "input " << index << " not ready";
            error_occurred(caller, out.str());
            return;
        }

        change_type(caller, result->type);
        assign_value(caller->inputs[index], caller);
    }

    Term* specializeType(Term* caller)
    {
        RefList choices(caller->input(1), caller->input(2));
        return find_common_type(choices);
    }

    void feedback_evaluate(Term* caller)
    {
        Term* target = caller->input(0);
        Term* desired = caller->input(1);
        Branch& output = as_branch(caller);

        // if_expr(cond, pos, neg)
        //
        // For cond, don't try to send feedback
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
        IF_EXPR_FUNC = import_function(kernel, evaluate, "if_expr(bool condition, any pos, any neg) : any; \"If 'condition' is true, returns 'pos'. Otherwise returns 'neg'. This function will probably get some special syntax in the future.\" end");
        function_t::get_specialize_type(IF_EXPR_FUNC) = specializeType;
        function_t::set_input_meta(IF_EXPR_FUNC, 1, true);
        function_t::set_input_meta(IF_EXPR_FUNC, 2, true);
        function_t::get_feedback_func(IF_EXPR_FUNC) =
            import_function(kernel, feedback_evaluate, "if_expr_feedback(any, any) : Branch");
    }
}
}
