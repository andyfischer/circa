// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace concat_function {

    void evaluate(Term* caller)
    {
        std::stringstream out;
        for (int index=0; index < caller->inputs.length(); index++) {
            out << as_string(caller->inputs[index]);
        }
        as_string(caller) = out.str();
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "concat(string...) : string");
        as_function(main_func).pureFunction = true;
    }
}
}
