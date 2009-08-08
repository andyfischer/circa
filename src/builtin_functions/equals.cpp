// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace equals_function {

    void evaluate(Term* caller)
    {
        Term *lhs = caller->input(0);
        Term *rhs = caller->input(1);

        if (!value_fits_type(rhs, lhs->type)) {
            error_occurred(caller, "Type mismatch");
            return;
        }

        Type &type = as_type(lhs->type);

        if (type.equals == NULL) {
            std::stringstream error;
            error << "type " << type.name << " has no equals function";
            error_occurred(caller, error.str());
            return;
        }

        as_bool(caller) = type.equals(lhs, rhs);
    }

    void evaluate_not(Term* caller)
    {
        evaluate(caller);

        if (!caller->hasError)
            as_bool(caller) = !as_bool(caller);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "equals(any,any) : bool");
        import_function(kernel, evaluate_not, "not_equals(any,any) : bool");
    }
}
}
