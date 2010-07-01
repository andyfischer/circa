// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

namespace assign_function {

    CA_FUNCTION(assign)
    {
        Term* root = INPUT_TERM(0);
        PathExpression path = get_lexpr_path_expression(root);
        root = path._head;
        TaggedValue* value = INPUT(1);

        copy(root, OUTPUT);
        touch(OUTPUT);
        assign_using_path(OUTPUT, path, value);
    }

    Term* specializeType(Term* term)
    {
        return parser::find_lexpr_root(term->input(0))->type;
    }

    void setup(Branch& kernel)
    {
        ASSIGN_FUNC = import_function(kernel, assign, "assign(any, any) -> any");
        function_t::get_attrs(ASSIGN_FUNC).specializeType = specializeType;
    }
}
}
