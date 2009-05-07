// Copyright 2008 Andrew Fischer

#include <circa.h>

// This function is used to change the type on an element
// It's called by the syntax: expression : type
// for example: [1,2] : Point

namespace circa {
namespace annotate_type_function {

    void evaluate(Term* caller)
    {
        if (!value_fits_type(caller->input(0), caller->input(1))) {
            error_occured(caller, "Type mismatch");
            return;
        }

        assign_value(caller->input(0), caller);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(1);
    }

    void setup(Branch& kernel)
    {
        ANNOTATE_TYPE_FUNC = import_function(kernel, evaluate, "annotate_type(any,Type) : any");
        as_function(ANNOTATE_TYPE_FUNC).specializeType = specializeType;
    }
}
}
