// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace type_check_function {

    void hosted_is_list(caStack* stack)
    {
        set_bool(circa_output(stack, 0), is_list(circa_input(stack, 0)));
    }
    void hosted_is_int(caStack* stack)
    {
        set_bool(circa_output(stack, 0), is_int(circa_input(stack, 0)));
    }
    void hosted_is_float(caStack* stack)
    {
        set_bool(circa_output(stack, 0), is_float(circa_input(stack, 0)));
    }
    void hosted_is_bool(caStack* stack)
    {
        set_bool(circa_output(stack, 0), is_bool(circa_input(stack, 0)));
    }
    void hosted_is_string(caStack* stack)
    {
        set_bool(circa_output(stack, 0), is_string(circa_input(stack, 0)));
    }
    void hosted_is_null(caStack* stack)
    {
        set_bool(circa_output(stack, 0), is_null(circa_input(stack, 0)));
    }
    void hosted_is_function(caStack* stack)
    {
        set_bool(circa_output(stack, 0), is_func(circa_input(stack, 0)));
    }
    void hosted_is_type(caStack* stack)
    {
        set_bool(circa_output(stack, 0), is_type(circa_input(stack, 0)));
    }
    void inputs_fit_function(caStack* stack)
    {
        caValue* inputs = circa_input(stack, 0);
        Term* function = circa_caller_input_term(stack, 1);
        Block* functionContents = function_contents(function);
        caValue* result = circa_output(stack, 0);

        bool varArgs = has_variable_args(functionContents);

        // Fail if wrong # of inputs
        if (!varArgs && (count_input_placeholders(functionContents) != circa_count(inputs))) {
            set_bool(result, false);
            return;
        }

        // Check each input
        for (int i=0; i < circa_count(inputs); i++) {
            Term* placeholder = get_effective_input_placeholder(functionContents, i);
            caValue* value = circa_index(inputs, i);
            if (value == NULL)
                continue;
            if (!cast_possible(value, declared_type(placeholder))) {
                set_bool(result, false);
                return;
            }
        }

        set_bool(result, true);
    }
    void overload_error_no_match(caStack* stack)
    {
        caValue* inputs = circa_input(stack, 00);

        Term* caller = (Term*) circa_caller_term(stack);
        Term* func = get_parent_term(caller, 3);

        std::stringstream out;
        out << "In overloaded function ";
        if (func == NULL)
            out << "<name not found>";
        else
            out << func->name;
        out << ", no func could handle inputs: ";
        out << to_string(inputs);
        circa_output_error(stack, out.str().c_str());
    }

    void setup(Block* kernel)
    {
        import_function(kernel, hosted_is_list, "is_list(any v) -> bool");
        import_function(kernel, hosted_is_int, "is_int(any v) -> bool");
        import_function(kernel, hosted_is_float, "is_float(any v) -> bool");
        import_function(kernel, hosted_is_bool, "is_bool(any v) -> bool");
        import_function(kernel, hosted_is_string, "is_string(any v) -> bool");
        import_function(kernel, hosted_is_null, "is_null(any v) -> bool");
        import_function(kernel, hosted_is_function, "is_function(any v) -> bool");
        import_function(kernel, hosted_is_type, "is_type(any v) -> bool");

        FUNCS.inputs_fit_function = import_function(kernel, inputs_fit_function,
            "inputs_fit_function(List inputs,Function func) -> bool");
        FUNCS.overload_error_no_match = import_function(kernel,
            overload_error_no_match, "overload_error_no_match(List inputs)");
    }

} // namespace type_check_function
} // namespace circa
