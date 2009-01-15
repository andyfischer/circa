// Copyright 2008 Andrew Fischer

#include "circa.h"
#include "compilation.h"

namespace circa {
namespace evaluate_file_function {

    void evaluate(Term* caller)
    {
        std::string &filename = as_string(caller->input(0));
        dealloc_value(caller);
        caller->value = evaluate_file(filename);
    }

    void setup(Branch& kernel)
    {
        /*Term* main_func = */import_function(kernel, evaluate,
                "function evaluate-file(string) -> Branch");
    }
}
} // namespace circa
