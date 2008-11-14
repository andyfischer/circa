// Copyright 2008 Andrew Fischer

namespace or_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = as_bool(caller->inputs[0]) || as_bool(caller->inputs[1]);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function or(bool,bool) -> bool");
        as_function(main_func).pureFunction = true;
    }
}
