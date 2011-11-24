// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "types/ref.h"

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

    Term* statically_specialize_function(Term* func, TermList const& inputs)
    {
        if (!is_function(func))
            return func;
        if (!is_overloaded_function(func))
            return func;

        List& overloads = as_function(func)->parameters;

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
        Branch* contents = nested_contents(CALLER);
        Term* func = CALLER->function;
        Function* funcAttrs = as_function(func);

        List& overloads = as_function(func)->parameters;

        int numInputs = NUM_INPUTS;

        // Dynamically specialize this function
        Term* specializedFunc = NULL;
        for (int i=0; i < overloads.length(); i++) {
            Term* overload = as_ref(overloads[i]);

            // Fail if wrong # of inputs
            if (!funcAttrs->variableArgs && (function_num_inputs(funcAttrs) != numInputs))
                continue;

            // Check each input
            bool inputsMatch = true;
            for (int i=0; i < numInputs; i++) {
                Type* type = function_get_input_type(overload, i);
                TaggedValue* value = INPUT(i);
                if (value == NULL)
                    continue;
                if (!cast_possible(value, type)) {
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
            bool alreadyGenerated = (contents->length() > 0)
                && contents->get(0)->function == specializedFunc;
            if (!alreadyGenerated) {
                TermList inputs;
                CALLER->inputsToList(inputs);
                apply(contents, specializedFunc, inputs);
                //change_declared_type(CALLER, contents[0]->type);
            }
            TaggedValue output;
            evaluate_branch_internal(CONTEXT, contents, &output);
            cast(&output, contents->get(0)->type, OUTPUT);
        } else {
            std::stringstream msg;
            msg << "specialized func not found for: " << CALLER->function->name;
            return ERROR_OCCURRED(msg.str().c_str());
        }
    }

    CA_FUNCTION(evaluate_overload)
    {
        evaluate_subroutine(_cxt, _ins, _outs);
#if 0
        Branch* contents = nested_contents(CALLER);
        if (contents->length() == 0) {
            //FIXME
            //evaluate_dynamic_overload(CONTEXT, CALLER);
            contents->clear();
        } else {
            TaggedValue output;
            evaluate_branch_internal(CONTEXT, contents, &output);
            if (OUTPUT != NULL)
                swap(&output, OUTPUT);
        }
#endif
    }

    void overload_post_input_change(Term* term)
    {
        Branch* contents = nested_contents(term);
        contents->clear();

        TermList inputs;
        term->inputsToList(inputs);
        Term* specializedFunc = statically_specialize_function(term->function, inputs);

        if (specializedFunc != NULL) {
            Term* result = apply(contents, specializedFunc, inputs);
            repoint_outer_inputs_to_new_placeholders(contents);
            change_declared_type(term, contents->get(0)->type);
            apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(result));
        }
        //dump(contents);
    }

    Type* overload_specialize_type(Term* term)
    {
        Branch* contents = nested_contents(term);
        return contents->get(0)->type;
    }

    bool is_overloaded_function(Term* func)
    {
        ca_assert(is_function(func));
        return as_function(func)->evaluate == evaluate_overload;
    }

    int num_overloads(Term* func)
    {
        return as_function(func)->parameters.length();
    }

    Term* get_overload(Term* func, int index)
    {
        return as_function(func)->parameters[index]->asRef();
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
        List& parameters = as_function(term)->parameters;

        ca_assert(parameters.length() > 0);
        int argumentCount = function_num_inputs(as_function(as_ref(parameters[0])));
        bool variableArgs = false;
        List outputTypes;

        for (int i=0; i < parameters.length(); i++) {

            Term* overload = as_ref(parameters[i]);
            Function* overloadAttrs = as_function(overload);

            if (argumentCount != function_num_inputs(overloadAttrs))
                variableArgs = true;
            if (overloadAttrs->variableArgs)
                variableArgs = true;
            set_type(outputTypes.append(), function_get_output_type(overload, 0));
        }

        Branch* result = nested_contents(term);
        clear_branch(result);
        int placeholderCount = variableArgs ? 1 : argumentCount;
        for (int i=0; i < placeholderCount; i++) {
            Term* term = apply(result, INPUT_PLACEHOLDER_FUNC, TermList());
            if (variableArgs)
                term->setBoolProp("multiple", true);
        }
        Type* outputType = find_common_type(&outputTypes);
        Function* func = as_function(term);
        func->variableArgs = variableArgs;
        finish_building_function(func, outputType);
    }

    void setup_overloaded_function(Term* term, std::string const& name,
            TermList const& overloads)
    {
        nested_contents(term);
        initialize_function(term);

        Function* attrs = as_function(term);
        attrs->name = name;
        attrs->evaluate = evaluate_overload;
        attrs->postInputChange = overload_post_input_change;
        attrs->createsStackFrame = true;
        // attrs->specializeType = overload_specialize_type;

        List& parameters = as_function(term)->parameters;
        parameters.clear();
        parameters.resize(overloads.length());

        for (int i=0; i < overloads.length(); i++)
            set_ref(parameters[i], overloads[i]);

        update_function_signature(term);
    }

    Term* create_overloaded_function(Branch* branch, std::string const& name,
        TermList const& overloads)
    {
        Term* result = create_value(branch, &FUNCTION_T, name);
        setup_overloaded_function(result, name, overloads);
        return result;
    }

    void overloaded_func_post_compile(Term* term)
    {
        TermList inputs;
        term->inputsToList(inputs);
        setup_overloaded_function(term, term->name, inputs);
    }

    void append_overload(Term* overloadedFunction, Term* overload)
    {
        ca_assert(is_overloaded_function(overloadedFunction));

        List& parameters = as_function(overloadedFunction)->parameters;
        set_ref(parameters.append(), overload);
        update_function_signature(overloadedFunction);
    }

    CA_FUNCTION(evaluate_declaration)
    {
        // Make our output of type Function so that the type checker doesn't
        // get mad. This value isn't used.
        create(unbox_type(FUNCTION_TYPE), OUTPUT);
    }

    void setup(Branch* kernel)
    {
        OVERLOADED_FUNCTION_FUNC = import_function(kernel, evaluate_declaration,
                "overloaded_function(Function...) -> Function");
        as_function(OVERLOADED_FUNCTION_FUNC)->postCompile = overloaded_func_post_compile;
    }
}
}
