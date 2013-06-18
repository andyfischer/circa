// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace comment_function {

    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, term->stringProp("comment",""), term, tok_Comment);
    }

    void setup(Block* kernel)
    {
        FUNCS.comment = import_function(kernel, NULL, "comment()");
        block_set_format_source_func(function_contents(FUNCS.comment), formatSource);
        block_set_evaluation_empty(function_contents(FUNCS.comment), true);
    }
}
}
