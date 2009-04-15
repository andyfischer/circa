// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace concat_function {

    void evaluate(Term* caller)
    {
        std::stringstream out;
        for (unsigned int index=0; index < caller->inputs.count(); index++) {
            out << as_string(caller->inputs[index]);
        }
        as_string(caller) = out.str();
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "concat(string,string) -> string");
        as_function(main_func).pureFunction = true;
        as_function(main_func).variableArgs = true;
    }
}
}
