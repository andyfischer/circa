// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace do_once_function {

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "do once", term, phrase_type::KEYWORD);
        format_branch_source(source, term->nestedContents, term);
        append_phrase(source, term->stringPropOptional("syntax:preEndWs", ""),
                term, token::WHITESPACE);
    }

    void setup(Branch& kernel)
    {
        DO_ONCE_FUNC = import_function(kernel, NULL, "do_once(state bool)");
        get_function_attrs(DO_ONCE_FUNC)->formatSource = formatSource;
    }
}
}
