// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace comment_function {

    void evaluate(EvalContext*, Term* caller)
    {
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, term->stringProp("comment"), term, token::COMMENT);
    }

    void setup(Branch& kernel)
    {
        COMMENT_FUNC = import_function(kernel, evaluate, "comment()");
        function_t::get_attrs(COMMENT_FUNC).formatSource = formatSource;
    }
}
}
