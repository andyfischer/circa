// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace unrecognized_expr_function {

    CA_FUNCTION(evaluate)
    {
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, term->stringProp("originalText"), term, phrase_type::UNDEFINED);
    }

    void setup(Branch& kernel)
    {
        UNRECOGNIZED_EXPRESSION_FUNC = import_function(kernel, evaluate, "unrecognized_expr()");
        function_t::get_attrs(UNRECOGNIZED_EXPRESSION_FUNC).formatSource = formatSource;
    }
}
}
