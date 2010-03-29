// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace stateful_value_function {

    void evaluate(EvalContext*, Term* caller)
    {
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "state ", term, token::STATE);

        if (term->hasProperty("syntax:explicitType")) {
            append_phrase(source, term->stringProp("syntax:explicitType"),
                    term, phrase_type::TYPE_NAME);
            append_phrase(source, " ", term, token::WHITESPACE);
        }

        append_phrase(source, term->name.c_str(), term, phrase_type::TERM_NAME);

        if (term->hasProperty("initializedBy")) {
            Term* initializedBy = term->refProp("initializedBy");
            append_phrase(source, " = ", term, phrase_type::UNDEFINED);
            if (initializedBy->name != "")
                append_phrase(source, get_relative_name(term, initializedBy),
                        term, phrase_type::TERM_NAME);
            else
                format_term_source(source, initializedBy);
        }
    }

    void setup(Branch& kernel)
    {
        STATEFUL_VALUE_FUNC = import_function(kernel, evaluate, "stateful_value() -> any");
        function_t::get_attrs(STATEFUL_VALUE_FUNC).formatSource = formatSource;
    }
}
}
