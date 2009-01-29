// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace generate_cpp_function_decl_function {

    void evaluate(Term* caller)
    {
        Function& func = as_function(caller->input(0));
        as_string(caller) = function_decl_to_cpp(func);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function generate-cpp-function-decl(Function) -> string");
        as_function(main_func).pureFunction = true;
    }
}
}
