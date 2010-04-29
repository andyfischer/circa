// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace overloaded_function {

    bool is_overloaded_function(Term* func);

    void evaluate_overloaded(EvalContext* cxt, Term* caller)
    {
        Term* func = caller->function;
        List& overloads = function_t::get_attrs(func).parameters;

        // Dynamically find the right overload.
        for (int i=0; i < overloads.length(); i++) {
            Term* overload = as_ref(overloads[i]);

            if (inputs_fit_function_dynamic(overload, caller->inputs)) {
                EvaluateFunc evaluate = function_t::get_evaluate(overload);
                return evaluate(cxt, caller);
            }
        }
    }

    Term* statically_specialize_function(Term* func, RefList const& inputs)
    {
        if (!is_overloaded_function(func))
            return func;

        List& overloads = function_t::get_attrs(func).parameters;

        for (int i=0; i < overloads.length(); i++) {
            Term* overload = as_ref(overloads[i]);

            if (inputs_fit_function(overload, inputs))
                return overload;
        }

        // no overload found
        return func;
    }

    bool is_overloaded_function(Term* func)
    {
        return function_t::get_attrs(func).evaluate == evaluate_overloaded;
    }

    void evaluate(EvalContext*, Term* caller)
    {
        function_t::set_name(caller, caller->name);
        function_t::get_attrs(caller).evaluate = evaluate_overloaded;

        List& parameters = function_t::get_attrs(caller).parameters;
        parameters.clear();
        parameters.resize(caller->numInputs());

        for (int i=0; i < caller->numInputs(); i++) {
            make_ref(parameters[i], caller->input(i));
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
                "overloaded_function(Function...) -> Function");
    }
}
}
