// Copyright (c) 2007-2009 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace meta_function {

    Term* freeze_specializeType(Term* term)
    {
        return term->input(0)->type;
    }

    void lift_closure_evaluate(Term* caller)
    {
        assign_value(caller->input(0), caller);
        lift_closure(as_branch(caller));
    }

    void setup(Branch& kernel)
    {
        FREEZE_FUNC = import_function(kernel, copy_function::evaluate, "freeze(any) -> any");
        function_t::get_specialize_type(FREEZE_FUNC) = freeze_specializeType;

        import_function(kernel, lift_closure_evaluate, "lift_closure(Branch) -> Branch");
    }

} // namespace meta_function
} // namespace circa
