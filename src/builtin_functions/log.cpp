// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace log_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = std::log(to_float(caller->input(0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "log(number) : number;"
            "'Natural log function' end");

    }
}
}
