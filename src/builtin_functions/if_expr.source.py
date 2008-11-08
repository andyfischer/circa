# Copyright 2008 Paul Hodge

header = "function if-expr(bool,any,any) -> any"
pure = True
evaluate = """
        int index = as_bool(caller->inputs[0]) ? 1 : 2;
        Term *result = caller->inputs[index];
        change_type(caller, result->type);
        recycle_value(caller->inputs[index], caller);
"""
