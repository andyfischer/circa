// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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
        function_t::get_specialize_type(ALIAS_FUNC) = specializeType;
        hide_from_docs(ALIAS_FUNC);
    }
}
}
