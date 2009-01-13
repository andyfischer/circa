// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace get_infix_function_name_function {

    void evaluate(Term* caller)
    {
        std::string& infix = as_string(caller->inputs[0]);
        std::string& output = as_string(caller);

        if (infix == "+")
            output = "add";
        else if (infix == "-")
            output = "sub";
        else if (infix == "*")
            output = "mult";
        else if (infix == "/")
            output = "div";
        else if (infix == "<")
            output = "less-than";
        else if (infix == "<=")
            output = "less-than-eq";
        else if (infix == ">")
            output = "greater-than";
        else if (infix == ">=")
            output = "greater-than-eq";
        else if (infix == "==")
            output = "equals";
        else if (infix == "||")
            output = "or";
        else if (infix == "&&")
            output = "and";
        else
            error_occured(caller, infix + " not recognized");
    }
        
    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "function get-infix-function-name(string) -> string");
    }
}
}
