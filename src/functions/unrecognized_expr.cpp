// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace unrecognized_expr_function {

    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, term->stringProp("originalText",""), term, sym_None);
    }

    void setup(Block* kernel)
    {
        FUNCS.unrecognized_expression = import_function(kernel, NULL,
                "unrecognized_expr(any ins :multiple)");
        as_function(FUNCS.unrecognized_expression)->formatSource = formatSource;
    }
}
}
