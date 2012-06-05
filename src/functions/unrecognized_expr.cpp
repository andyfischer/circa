// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace unrecognized_expr_function {

    CA_FUNCTION(evaluate)
    {
    }

    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, term->stringProp("originalText"), term, phrase_type::UNDEFINED);
    }

    void setup(Branch* kernel)
    {
        UNRECOGNIZED_EXPRESSION_FUNC = import_function(kernel, evaluate, "unrecognized_expr(any :multiple)");
        as_function(UNRECOGNIZED_EXPRESSION_FUNC)->formatSource = formatSource;
    }
}
}
