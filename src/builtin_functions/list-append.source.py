# Copyright 2008 Andrew Fischer

header = "function list-append(List, any) -> List"
pure = True
evaluate = """
    recycle_value(caller->inputs[0], caller);
    as_list(caller).append(caller->inputs[1]);
"""
