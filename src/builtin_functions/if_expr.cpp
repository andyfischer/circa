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
            error_occured(caller, out.str());
            return;
        }

        change_type(caller, result->type);
        assign_value(caller->inputs[index], caller);
    }

    Term* specializeType(Term* caller)
    {
        // if inputs 1 and 2 have the same type, we can use that
        if (caller->input(1)->type == caller->input(2)->type)
            return caller->input(1)->type;

        // Otherwise return any. We might want to signal an error here
        return ANY_TYPE;
    }

    void generateFeedback(Branch& branch, Term* subject, Term* desired)
    {
        if (as_bool(subject->input(0)))
            generate_feedback(branch, subject->input(1), desired);
        else
            generate_feedback(branch, subject->input(2), desired);
    }

    void setup(Branch& kernel)
    {
        IF_EXPR_FUNC = import_function(kernel, evaluate, "if_expr(bool,any,any) : any");
        as_function(IF_EXPR_FUNC).specializeType = specializeType;
        as_function(IF_EXPR_FUNC).pureFunction = true;
        as_function(IF_EXPR_FUNC).setInputMeta(1, true);
        as_function(IF_EXPR_FUNC).setInputMeta(2, true);
        as_function(IF_EXPR_FUNC).generateFeedback = generateFeedback;
    }
}
}
