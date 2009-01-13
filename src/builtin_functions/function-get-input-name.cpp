// Copyright 2008 Paul Hodge

#include "circa.h"
#include "function.h"

namespace circa {
namespace function_get_input_name_function {

    void evaluate(Term* caller)
    {
        Function& sub = as_function(caller->input(0));
        int index = as_int(caller->input(1));

        Term* inputPlaceholder =
            sub.subroutineBranch.getNamed(get_placeholder_name_for_index(index));

        as_string(caller) = inputPlaceholder->name;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "function-get-input-name(Function, int) -> string");
    }
}
}
