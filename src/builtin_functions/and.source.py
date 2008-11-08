# Copyright 2008 Paul Hodge

header = "function and(bool,bool) -> bool"
pure = True
evaluate = "as_bool(caller) = as_bool(caller->inputs[0]) && as_bool(caller->inputs[1]);"
