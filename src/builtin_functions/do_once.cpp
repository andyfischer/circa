// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace do_once_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        Term* done = caller->input(0);

        if (!as_bool(done)) {
            evaluate_branch(cxt, caller->asBranch());
            set_bool(done, true);
        }
    }

    void formatSource(RichSource* source, Term* term)
    {
        append_phrase(source, "do once", term, phrase_type::KEYWORD);
        append_phrase(source, term->stringPropOptional("syntax:postHeadingWs", "\n"),
                term, token::WHITESPACE);
        format_branch_source(source, as_branch(term), NULL);
        append_phrase(source, term->stringPropOptional("syntax:preEndWs", ""),
                term, token::WHITESPACE);
                
        append_phrase(source, "end", term, phrase_type::KEYWORD);
    }

    void setup(Branch& kernel)
    {
        DO_ONCE_FUNC = import_function(kernel, evaluate, "do_once(state bool) -> Code");
        function_t::get_attrs(DO_ONCE_FUNC).formatSource = formatSource;
    }
}
}
