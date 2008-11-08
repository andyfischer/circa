
// Copyright 2008 Andrew Fischer

// This file was generated using ../src/builtin_functions/and.source.py. You should probably not modify
// this file directly.

namespace and_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = as_bool(caller->inputs[0]) && as_bool(caller->inputs[1]);
    }

    
    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function and(bool,bool) -> bool");
        as_function(main_func).pureFunction = true;

        
    }
}
