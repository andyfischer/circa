
// Copyright 2008 Paul Hodge

// This file was generated using ../src/builtin_functions/if_expr.source.py. You should probably not modify
// this file directly.

namespace if_expr_function {

    void evaluate(Term* caller)
    {
        
        int index = as_bool(caller->inputs[0]) ? 1 : 2;
        Term *result = caller->inputs[index];
        change_type(caller, result->type);
        recycle_value(caller->inputs[index], caller);

    }

    
    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function if-expr(bool,any,any) -> any");
        as_function(main_func).pureFunction = true;

        
    }
}
