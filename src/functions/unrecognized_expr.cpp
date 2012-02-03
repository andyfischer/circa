// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa_internal.h"

namespace circa {
namespace unrecognized_expr_function {

    CA_FUNCTION(evaluate)
    {
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, term->stringProp("originalText"), term, phrase_type::UNDEFINED);
    }

    void setup(Branch* kernel)
    {
        UNRECOGNIZED_EXPRESSION_FUNC = import_function(kernel, evaluate, "unrecognized_expr(any...)");
        as_function(UNRECOGNIZED_EXPRESSION_FUNC)->formatSource = formatSource;
    }
}
}
