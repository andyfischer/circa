// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace equals_function {

    void evaluate(EvalContext*, Term* caller)
    {
        Term *lhs = caller->input(0);
        Term *rhs = caller->input(1);

        if (!value_fits_type(rhs, lhs->type)) {
            error_occurred(caller, "Type mismatch");
            return;
        }

        set_bool(caller, equals(lhs, rhs));
    }

    void evaluate_not(EvalContext* cxt, Term* caller)
    {
        evaluate(cxt, caller);

        if (!caller->hasError())
            set_bool(caller, !as_bool(caller));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "equals(any,any) -> bool");
        import_function(kernel, evaluate_not, "not_equals(any,any) -> bool");
    }
}
}
