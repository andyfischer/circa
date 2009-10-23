// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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

        EqualsFunc equals = type_t::get_equals_func(lhs->type);

        if (equals == NULL) {
            std::stringstream error;
            error << "type " << type_t::get_name(lhs->type) << " has no equals function";
            error_occurred(caller, error.str());
            return;
        }

        as_bool(caller) = equals(lhs, rhs);
    }

    void evaluate_not(Term* caller)
    {
        evaluate(caller);

        if (!caller->hasError())
            as_bool(caller) = !as_bool(caller);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "equals(any,any) :: bool");
        import_function(kernel, evaluate_not, "not_equals(any,any) :: bool");
    }
}
}
