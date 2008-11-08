# Copyright 2008 Andrew Fischer

header = "function or(bool,bool) -> bool"
pure = True
evaluate = "as_bool(caller) = as_bool(caller->inputs[0]) || as_bool(caller->inputs[1]);"
