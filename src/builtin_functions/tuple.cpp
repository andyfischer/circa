// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace tuple_function {

    void evaluate(Term* caller)
    {
        as<RefList>(caller) = caller->inputs;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "tuple(any...) : Tuple");
    }
}

} // namespace circa
