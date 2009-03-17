// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace if_expr_function {

    void evaluate(Term* caller)
    {
        int index = as_bool(caller->input(0)) ? 1 : 2;
        Term *result = caller->inputs[index];

        if (result->needsUpdate || !is_value_alloced(result)) {
            std::stringstream out;
            out << "input " << index << " not ready";
            error_occured(caller, out.str());
            return;
        }

        change_type(caller, result->type);
        recycle_value(caller->inputs[index], caller);
    }

    void generateTraining(Branch& branch, Term* subject, Term* desired)
    {
        if (as_bool(subject->input(0)))
            generate_training(branch, subject->input(1), desired);
        else
            generate_training(branch, subject->input(2), desired);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function if-expr(bool,any,any) -> any");
        as_function(main_func).pureFunction = true;
        as_function(main_func).setInputMeta(1, true);
        as_function(main_func).setInputMeta(2, true);
        as_function(main_func).generateTraining = generateTraining;
    }
}
}
