// Copyright 2008 Andrew Fischer

namespace or_function {

    void evaluate(Term* caller)
    {
        as_bool(caller) = as_bool(caller->inputs[0]) || as_bool(caller->inputs[1]);
    }

    void setup(Branch& kernel)
    {
        import_c_function(kernel, evaluate,
            "function or(bool, bool) -> bool");
    }
}
