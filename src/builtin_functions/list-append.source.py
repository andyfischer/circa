# Copyright 2008 Paul Hodge

header = "function list-append(List, any) -> List"
pure = True
evaluate = """
    recycle_value(caller->inputs[0], caller);
    as_list(caller).append(caller->inputs[1]);
"""
