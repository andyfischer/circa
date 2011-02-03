// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

// alias() is used to describe a value that should always be the same as the input
// value, and no copy is needed. In the future we'll perform optimizations based
// on this assumption.

namespace circa {
namespace alias_function {

    CA_FUNCTION(alias)
    {
        copy(INPUT(0), OUTPUT);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void setup(Branch& kernel)
    {
        ALIAS_FUNC = import_function(kernel, alias, "alias(any) -> any");
        get_function_attrs(ALIAS_FUNC)->specializeType = specializeType;
        hide_from_docs(ALIAS_FUNC);
    }
}
}
