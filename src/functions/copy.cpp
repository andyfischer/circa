// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa_internal.h"

namespace circa {
namespace copy_function {

    CA_FUNCTION(evaluate)
    {
        copy(INPUT(0), OUTPUT);
    }

    Type* specializeType(Term* caller)
    {
        return get_type_of_input(caller, 0);
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, get_relative_name_at(term, term->input(0)),
                term, TK_IDENTIFIER);
    }

    void setup(Branch* kernel)
    {
        COPY_FUNC = import_function(kernel, evaluate, "copy(any) -> any");
        as_function(COPY_FUNC)->specializeType = specializeType;
        as_function(COPY_FUNC)->formatSource = formatSource;
    }
}
}
