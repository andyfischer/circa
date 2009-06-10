// Copyright 2008 Paul Hodge

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
        Term* left_type = caller->input(1)->type;
        Term* right_type = caller->input(2)->type;

        // if inputs 1 and 2 have the same type, we can use that
        if (left_type == right_type)
            return left_type;

        // special case if one input is int and another is float
        if ((left_type == INT_TYPE || left_type == FLOAT_TYPE)
            && (right_type == INT_TYPE || right_type == FLOAT_TYPE))
            return FLOAT_TYPE;


        // Otherwise return any. (we might want to signal an error here)
        return ANY_TYPE;
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
        IF_EXPR_FUNC = import_function(kernel, evaluate, "if_expr(bool,any,any) : any");
        as_function(IF_EXPR_FUNC).specializeType = specializeType;
        as_function(IF_EXPR_FUNC).pureFunction = true;
        as_function(IF_EXPR_FUNC).setInputMeta(1, true);
        as_function(IF_EXPR_FUNC).setInputMeta(2, true);
        as_function(IF_EXPR_FUNC).feedbackFunc = 
            import_function(kernel, feedback_evaluate, "if_expr_feedback(any, any) : Branch");
    }
}
}
