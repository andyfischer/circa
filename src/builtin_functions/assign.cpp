// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

namespace assign_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        Term* root = caller->input(0);
        PathExpression path = get_lexpr_path_expression(root);
        root = path._head;
        TaggedValue* value = caller->input(1);

        copy(root, caller);
        mutate(caller);
        assign_using_path(caller, path, value);
    }

    Term* specializeType(Term* term)
    {
        return parser::find_lexpr_root(term->input(0))->type;
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, evaluate, "assign(any, any) -> any");
        function_t::get_attrs(ASSIGN_FUNC).specializeType = specializeType;
    }
}
}
