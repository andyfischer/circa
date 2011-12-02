// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

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
    CA_FUNCTION(typeof_func)
    {
        set_type(OUTPUT, declared_type(INPUT_TERM(0)));
    }
    CA_FUNCTION(typename_func)
    {
        set_string(OUTPUT, declared_type(INPUT_TERM(0))->name);
    }
    CA_FUNCTION(inputs_fit_function)
    {
        List* inputs = as_list(INPUT(0));
        Function* function = as_function(INPUT(1));

        bool varArgs = function_has_variable_args(function);

        // Fail if wrong # of inputs
        if (!varArgs && (function_num_inputs(function) != inputs->length())) {
            set_bool(OUTPUT, false);
            return;
        }

        // Check each input
        for (int i=0; i < inputs->length(); i++) {
            Type* type = function_get_input_type(function, i);
            TaggedValue* value = inputs->get(i);
            if (value == NULL)
                continue;
            if (!cast_possible(value, type)) {
                set_bool(OUTPUT, false);
                return;
            }
        }

        set_bool(OUTPUT, true);
    }
    CA_FUNCTION(overload_error_no_match)
    {
        List* inputs = as_list(INPUT(0));

        Term* caller = CALLER;
        Term* func = get_parent_term(caller, 3);

        std::stringstream out;
        out << "In overloaded function ";
        if (func == NULL)
            out << "<name not found>";
        else
            out << func->name;
        out << ", no func could handle inputs: ";
        out << inputs->toString();
        ERROR_OCCURRED(out.str().c_str());
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        BUILTIN_FUNCS.inputs_fit_function = import_function(kernel, inputs_fit_function,
            "inputs_fit_function(List,Function) -> bool");
        BUILTIN_FUNCS.overload_error_no_match = import_function(kernel,
            overload_error_no_match, "overload_error_no_match(List)");
        import_function(kernel, typeof_func, "type(any :meta) -> Type");
        import_function(kernel, typename_func, "typename(any :meta) -> string");
    }

} // namespace type_check_function
} // namespace circa
