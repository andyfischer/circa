// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace unique_id_function {

    void evaluate(Term* caller)
    {
        static int nextId = 1;
        as_int(caller) = nextId++;
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate, "unique_id() : int");
        as_function(main_func).pureFunction = false;
    }
}
}
