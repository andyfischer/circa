// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace log_function {

    CA_FUNCTION(evaluate)
    {
        set_float(OUTPUT, std::log(FLOAT_INPUT(0)));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "log(number) -> number;"
            "'Natural log function' end");
    }
}
}
