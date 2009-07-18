// Copyright 2008 Paul Hodge

#include <circa.h>

// This function is used to change the type on an element
// It's called by the syntax: expression : type
// for example: [1,2] : Point

namespace circa {
namespace annotate_type_function {

    void evaluate(Term* caller)
    {
        if (!value_fits_type(caller->input(0), caller->input(1))) {
            std::stringstream message;
            message << "A value of type " << caller->input(0)->type->name;
            message << " doesn't fit in type " << caller->input(1)->name;
            error_occurred(caller, message.str());
            return;
        }

        assign_value(caller->input(0), caller);
    }

    Term* specializeType(Term* caller)
    {
        Term* type = caller->input(1);
        if (!is_type(type))
            return ANY_TYPE;
        else
            return type;
    }

    void setup(Branch& kernel)
    {
        ANNOTATE_TYPE_FUNC = import_function(kernel, evaluate, "annotate_type(any,Type) : any");
        function_get_specialize_type(ANNOTATE_TYPE_FUNC) = specializeType;
    }
}
}
