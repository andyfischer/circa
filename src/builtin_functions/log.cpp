// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace log_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::log(as_float(caller->input(0)));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "log(float) : float");
        as_function(main_func).pureFunction = true;
    }
}
}
