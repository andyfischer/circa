# Copyright 2008 Andrew Fischer

header = "function mult(float,float) -> float"
pure = True
evaluate = """
    as_float(caller) = as_float(caller->inputs[0]) * as_float(caller->inputs[1]);
"""
