# Copyright 2008 Paul Hodge

header = "function range(int) -> List"
pure = True
evaluate = """
    unsigned int max = as_int(caller->inputs[0]);

    as_list(caller).clear();

    for (unsigned int i=0; i < max; i++) {
        as_list(caller).append(int_var(*caller->owningBranch, i));
    }
"""
