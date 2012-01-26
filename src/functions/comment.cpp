// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace comment_function {

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, term->stringProp("comment"), term, TK_COMMENT);
    }

    void setup(Branch* kernel)
    {
        COMMENT_FUNC = import_function(kernel, NULL, "comment()");
        as_function(COMMENT_FUNC)->formatSource = formatSource;
    }
}
}
