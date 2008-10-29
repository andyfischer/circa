// Copyright 2008 Andrew Fischer

namespace if_expr_function {

    void evaluate(Term* caller)
    {
        int index = as_bool(caller->inputs[0]) ? 1 : 2;
        Term *result = caller->inputs[index];
        change_type(caller, result->type);
        recycle_value(caller->inputs[index], caller);
    }

    void setup(Branch *kernel)
    {
        quick_create_function(kernel, "if-expr", evaluate,
            ReferenceList(BOOL_TYPE, ANY_TYPE, ANY_TYPE), ANY_TYPE);
    }
}
