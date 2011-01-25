// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace overloaded_function {

    bool is_overloaded_function(Term* func);

    Term* dynamically_specialize_func(List* overloadRefs, List* inputs)
    {
        for (int i=0; i < overloadRefs->length(); i++) {
            Term* overload = as_ref(overloadRefs->get(i));

            if (values_fit_function_dynamic(overload, inputs))
                return overload;
        }
        return NULL;
    }

    Term* statically_specialize_function(Term* func, RefList const& inputs)
    {
        if (!is_function(func))
            return func;
        if (!is_overloaded_function(func))
            return func;

        List& overloads = function_t::get_attrs(func).parameters;

        for (int i=0; i < overloads.length(); i++) {
            Term* overload = as_ref(overloads[i]);

            if (inputs_statically_fit_function(overload, inputs))
                return overload;
        }

        // no overload found
        return NULL;
    }

    CA_FUNCTION(evaluate_dynamic_overload)
    {
        Branch& contents = CALLER->nestedContents;
        Term* func = CALLER->function;

        List& overloads = function_t::get_attrs(func).parameters;

        int numInputs = NUM_INPUTS;

        // Dynamically specialize this function
        Term* specializedFunc = NULL;
        for (int i=0; i < overloads.length(); i++) {
            Term* overload = as_ref(overloads[i]);
            bool varArgs = function_t::get_variable_args(func);

            // Fail if wrong # of inputs
            if (!varArgs && (function_t::num_inputs(overload) != numInputs))
                continue;

            // Check each input
            bool inputsMatch = true;
            for (int i=0; i < numInputs; i++) {
                Type* type = unbox_type(function_t::get_input_type(overload, i));
                TaggedValue* value = INPUT(i);
                if (value == NULL)
                    continue;
                if (!value_fits_type(value, type)) {
                    inputsMatch = false;
                    break;
                }
            }

            if (!inputsMatch)
                continue;

            specializedFunc = overload;
            break;
        }

        if (specializedFunc != NULL) {
            bool alreadyGenerated = (contents.length() > 0)
                && contents[0]->function == specializedFunc;
            if (!alreadyGenerated) {
                apply(contents, specializedFunc, CALLER->inputs);
                //change_type(CALLER, contents[0]->type);
            }
            TaggedValue output;
            evaluate_branch_internal(CONTEXT, contents, &output);
            cast(&output, unbox_type(contents[0]->type), OUTPUT);
        } else {
            return error_occurred(CONTEXT, CALLER, "specialized func not found");
        }
    }

    CA_FUNCTION(evaluate_overload)
    {
        Branch& contents = CALLER->nestedContents;
        if (contents.length() == 0) {
            evaluate_dynamic_overload(CONTEXT, CALLER);
            contents.clear();
        } else {
            TaggedValue output;
            evaluate_branch_internal(CONTEXT, contents, &output);
            if (OUTPUT != NULL)
                swap(&output, OUTPUT);
        }
    }

    void overload_post_input_change(Term* term)
    {
        Branch& contents = term->nestedContents;
        contents.clear();

        Term* specializedFunc = statically_specialize_function(term->function, term->inputs);

        if (specializedFunc != NULL) {
            apply(contents, specializedFunc, term->inputs);
            change_type(term, contents[0]->type);
        }
    }

    Term* overload_specialize_type(Term* term)
    {
        Branch& contents = term->nestedContents;
        return contents[0]->type;
    }

    bool is_overloaded_function(Term* func)
    {
        ca_assert(is_function(func));
        return function_t::get_attrs(func).evaluate == evaluate_overload;
    }

    int num_overloads(Term* func)
    {
        return function_t::get_attrs(func).parameters.length();
    }

    Term* get_overload(Term* func, int index)
    {
        return function_t::get_attrs(func).parameters[index]->asRef();
    }

    Term* find_overload(Term* func, const char* name)
    {
        for (int i=0; i < num_overloads(func); i++) {
            Term* overload = get_overload(func, i);
            if (overload->name == name)
                return overload;
        }
        return NULL;
    }

    void update_function_signature(Term* term)
    {
        List& parameters = function_t::get_attrs(term).parameters;

        ca_assert(parameters.length() > 0);
        int argumentCount = function_t::num_inputs(as_ref(parameters[0]));
        bool variableArgs = false;
        RefList outputTypes;

        for (int i=0; i < parameters.length(); i++) {

            Term* overload = as_ref(parameters[i]);

            if (argumentCount != function_t::num_inputs(overload))
                variableArgs = true;
            if (function_t::get_variable_args(overload))
                variableArgs = true;
            outputTypes.append(function_t::get_output_type(overload));
        }

        Branch& result = term->nestedContents;
        result.shorten(1);
        int placeholderCount = variableArgs ? 1 : argumentCount;
        for (int i=0; i < placeholderCount; i++)
            apply(result, INPUT_PLACEHOLDER_FUNC, RefList());
        Term* outputType = find_common_type(outputTypes);
        function_t::set_output_type(term, outputType);
        function_t::set_variable_args(term, variableArgs);
    }

    void setup_overloaded_function(Term* term, std::string const& name,
            RefList const& overloads)
    {
        term->nestedContents.clear();
        initialize_function(term);

        function_t::set_name(term, name);
        function_t::get_attrs(term).evaluate = evaluate_overload;
        function_t::get_attrs(term).postInputChange = overload_post_input_change;
        //function_t::get_attrs(term).specializeType = overload_specialize_type;

        List& parameters = function_t::get_attrs(term).parameters;
        parameters.clear();
        parameters.resize(overloads.length());

        for (int i=0; i < overloads.length(); i++)
            set_ref(parameters[i], overloads[i]);

        update_function_signature(term);
    }

    Term* create_overloaded_function(Branch& branch, std::string const& name,
        RefList const& overloads)
    {
        Term* result = create_value(branch, FUNCTION_TYPE, name);
        setup_overloaded_function(result, name, overloads);
        return result;
    }

    void overloaded_func_post_compile(Term* term)
    {
        setup_overloaded_function(term, term->name, term->inputs);
    }

    void append_overload(Term* overloadedFunction, Term* overload)
    {
        ca_assert(is_overloaded_function(overloadedFunction));

        List& parameters = function_t::get_attrs(overloadedFunction).parameters;
        set_ref(parameters.append(), overload);
        update_function_signature(overloadedFunction);
    }

    CA_FUNCTION(evaluate_declaration)
    {
        // Make our output of type Function so that the type checker doesn't
        // get mad. This value isn't used.
        change_type(OUTPUT, unbox_type(FUNCTION_TYPE));
    }

    void setup(Branch& kernel)
    {
        OVERLOADED_FUNCTION_FUNC = import_function(kernel, evaluate_declaration,
                "overloaded_function(Function...) -> Function");
        get_function_attrs(OVERLOADED_FUNCTION_FUNC)->postCompile = overloaded_func_post_compile;
    }
}
}
