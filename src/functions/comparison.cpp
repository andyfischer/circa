// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace comparison_function {

    void less_than_i(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                circa_int_input(stack, 0) < circa_int_input(stack, 1));
    }

    void less_than_f(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                circa_float_input(stack, 0) < circa_float_input(stack, 1));
    }

    void less_than_eq_i(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                circa_int_input(stack, 0) <= circa_int_input(stack, 1));
    }

    void less_than_eq_f(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                circa_float_input(stack, 0) <= circa_float_input(stack, 1));
    }

    void greater_than_i(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                circa_int_input(stack, 0) > circa_int_input(stack, 1));
    }

    void greater_than_f(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                circa_float_input(stack, 0) > circa_float_input(stack, 1));
    }

    void greater_than_eq_i(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                circa_int_input(stack, 0) >= circa_int_input(stack, 1));
    }

    void greater_than_eq_f(caStack* stack)
    {
        set_bool(circa_output(stack, 0),
                circa_float_input(stack, 0) >= circa_float_input(stack, 1));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, less_than_i, "less_than_i(int a,int b) -> bool");
        import_function(kernel, less_than_f, "less_than_f(number a,number b) -> bool");
        import_function(kernel, less_than_eq_i, "less_than_eq_i(int a,int b) -> bool");
        import_function(kernel, less_than_eq_f, "less_than_eq_f(number a,number b) -> bool");
        import_function(kernel, greater_than_i, "greater_than_i(int a,int b) -> bool");
        import_function(kernel, greater_than_f, "greater_than_f(number a,number b) -> bool");
        import_function(kernel, greater_than_eq_i, "greater_than_eq_i(int a,int b) -> bool");
        import_function(kernel, greater_than_eq_f, "greater_than_eq_f(number a,number b) -> bool");
    }
}
}
