// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace do_once_function {

    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, "do once", term, name_Keyword);
        format_branch_source(source, nested_contents(term), term);
        append_phrase(source, term->stringPropOptional("syntax:preEndWs", ""),
                term, TK_WHITESPACE);
    }

    void setup(Branch* kernel)
    {
        DO_ONCE_FUNC = import_function(kernel, NULL, "do_once(state bool)");
        as_function(DO_ONCE_FUNC)->formatSource = formatSource;
    }
}
}
