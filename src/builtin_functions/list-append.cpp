// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace list_append_function {

    void evaluate(Term* caller)
    {
        recycle_value(caller->input(0), caller);
        as_list(caller).append(caller->input(1));
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function list-append(List, any) -> List");
        as_function(main_func).pureFunction = true;
    }
}
}
