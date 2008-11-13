# Copyright 2008 Andrew Fischer

header = "function concat(string,string) -> string"
pure = True
evaluate = """
    std::stringstream out;
    for (unsigned int index=0; index < caller->inputs.count(); index++) {
        out << as_string(caller->inputs[index]);
    }
    as_string(caller) = out.str();
"""
