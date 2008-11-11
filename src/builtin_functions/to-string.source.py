# Copyright 2008 Andrew Fischer

header = "function to-string(any) -> string"
pure = True
evaluate = """
        Term* term = caller->inputs[0];

        Type::ToStringFunc func = as_type(term->type).toString;

        if (func == NULL) {
            as_string(caller) = std::string("<" + as_type(term->type).name
                    + " " + term->name + ">");
        } else {
            as_string(caller) = func(term);
        }
"""
