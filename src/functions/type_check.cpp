// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace type_check_function {

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
        Value msg;
        string_append(&msg, "In overloaded function ");

        if (func == NULL)
            string_append(&msg, "<name not found>");
        else
            string_append(&msg, func->name);
        string_append(&msg, ", no func could handle inputs: ");
        string_append(&msg, inputs);
        circa_output_error(stack, as_cstring(&msg));
    }

    void setup(Block* kernel)
    {
        FUNCS.inputs_fit_function = import_function(kernel, inputs_fit_function,
            "inputs_fit_function(List inputs,Function func) -> bool");
        FUNCS.overload_error_no_match = import_function(kernel,
            overload_error_no_match, "overload_error_no_match(List inputs)");
    }

} // namespace type_check_function
} // namespace circa
