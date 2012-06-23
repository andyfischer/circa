// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace type_check_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(is_list, "is_list(any) -> bool")
    {
        set_bool(OUTPUT, circa::is_list(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_int, "is_int(any) -> bool")
    {
        set_bool(OUTPUT, is_int(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_float, "is_float(any) -> bool")
    {
        set_bool(OUTPUT, is_float(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_bool, "is_bool(any) -> bool")
    {
        set_bool(OUTPUT, is_bool(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_string, "is_string(any) -> bool")
    {
        set_bool(OUTPUT, is_string(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_null, "is_null(any) -> bool")
    {
        set_bool(OUTPUT, is_null(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_function, "is_function(any) -> bool")
    {
        set_bool(OUTPUT, is_function(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_type, "is_type(any) -> bool")
    {
        set_bool(OUTPUT, is_type(INPUT(0)));
    }
    void inputs_fit_function(caStack* stack)
    {
        caValue* inputs = circa_input(stack, 0);
        Function* function = as_function(circa_input(stack, 1));
        caValue* result = circa_output(stack, 0);

        bool varArgs = function_has_variable_args(function);

        // Fail if wrong # of inputs
        if (!varArgs && (function_num_inputs(function) != circa_count(inputs))) {
            set_bool(result, false);
            return;
        }

        // Check each input
        for (int i=0; i < circa_count(inputs); i++) {
            Type* type = function_get_input_type(function, i);
            caValue* value = circa_index(inputs, i);
            if (value == NULL)
                continue;
            if (!cast_possible(value, type)) {
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

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        FUNCS.inputs_fit_function = import_function(kernel, inputs_fit_function,
            "inputs_fit_function(List,Function) -> bool");
        FUNCS.overload_error_no_match = import_function(kernel,
            overload_error_no_match, "overload_error_no_match(List)");
    }

} // namespace type_check_function
} // namespace circa
