// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace copy_function {

    CA_FUNCTION(evaluate)
    {
        copy(INPUT(0), OUTPUT);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, get_relative_name(term, term->input(0)),
                term, token::IDENTIFIER);
    }

    void setup(Branch& kernel)
    {
        COPY_FUNC = import_function(kernel, evaluate, "copy(any) -> any");
        function_t::get_attrs(COPY_FUNC).specializeType = specializeType;
        function_t::get_attrs(COPY_FUNC).formatSource = formatSource;
    }
}
}
