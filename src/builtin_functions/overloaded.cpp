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
                Branch tempBranch;
                Term* tempTerm = apply(tempBranch, overload, caller->inputs);
                evaluate_branch(cxt, tempBranch);
                copy(tempTerm, caller);
                return;
            }
        }

        error_occurred(cxt, caller, "No usable overload found");
    }

    Term* statically_specialize_function(Term* func, RefList const& inputs)
    {
        if (!is_overloaded_function(func))
            return func;

        List& overloads = function_t::get_attrs(func).parameters;

        for (int i=0; i < overloads.length(); i++) {
            Term* overload = as_ref(overloads[i]);

            if (inputs_statically_fit_function(overload, inputs))
                return overload;
        }

        // no overload found
        return func;
    }

    bool is_overloaded_function(Term* func)
    {
        return function_t::get_attrs(func).evaluate == evaluate_overloaded;
    }

    void setup_overloaded_function(Term* term, std::string const& name,
            RefList const& overloads)
    {
        function_t::set_name(term, name);
        function_t::get_attrs(term).evaluate = evaluate_overloaded;

        List& parameters = function_t::get_attrs(term).parameters;
        parameters.clear();
        parameters.resize(overloads.length());

        assert(overloads.length() > 0);
        int argumentCount = function_t::num_inputs(overloads[0]);
        bool variableArgs = false;
        RefList outputTypes;

        for (int i=0; i < overloads.length(); i++) {
            make_ref(parameters[i], overloads[i]);

            if (argumentCount != function_t::num_inputs(overloads[i]))
                variableArgs = true;
            if (function_t::get_variable_args(overloads[i]))
                variableArgs = true;
            outputTypes.append(function_t::get_output_type(overloads[i]));
        }


        Branch& result = as_branch(term);
        result.shorten(1);
        for (int i=0; i < argumentCount; i++)
            apply(result, INPUT_PLACEHOLDER_FUNC, RefList());
        Term* outputType = find_common_type(outputTypes);
        create_value(result, outputType, "#out");

        function_t::set_variable_args(term, variableArgs);
    }

    void evaluate(EvalContext* cxt, Term* caller)
    {
        if (caller->numInputs() == 0)
            return error_occurred(cxt, caller, "Number of inputs must be >0");

        setup_overloaded_function(caller, caller->name, caller->inputs);

#if 0
        function_t::set_name(caller, caller->name);
        function_t::get_attrs(caller).evaluate = evaluate_overloaded;

        List& parameters = function_t::get_attrs(caller).parameters;
        parameters.clear();
        parameters.resize(caller->numInputs());

        int argumentCount = function_t::num_inputs(caller->input(0));

        for (int i=0; i < caller->numInputs(); i++) {
            make_ref(parameters[i], caller->input(i));

            if (argumentCount != function_t::num_inputs(caller->input(i)))
                return error_occurred(cxt, caller, "Functions must have the same # of inputs");
        }

        Branch& result = as_branch(caller);
        result.shorten(1);
        for (int i=0; i < argumentCount; i++)
            apply(result, INPUT_PLACEHOLDER_FUNC, RefList());
        create_value(result, VOID_TYPE, "#out");
#endif
    }

    Term* create_overloaded_function(Branch& branch, std::string const& name,
        RefList const& overloads)
    {
        Term* result = create_value(branch, FUNCTION_TYPE, name);
        setup_overloaded_function(result, name, overloads);
        return result;
    }

    void append_overload(Term* overloadedFunction, Term* overload)
    {
        assert(is_overloaded_function(overloadedFunction));

        List& parameters = function_t::get_attrs(overloadedFunction).parameters;
        make_ref(parameters.append(), overload);
    }

    void setup(Branch& kernel)
    {
        OVERLOADED_FUNCTION_FUNC = import_function(kernel, evaluate,
                "overloaded_function(Function...) -> Function");
    }
}
}
