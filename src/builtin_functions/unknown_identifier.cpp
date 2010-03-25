// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtins.h"
#include "circa.h"

namespace circa {
namespace unknown_identifier_function {

    void evaluate(EvalContext*, Term* caller)
    {
    }

    std::string toSourceString(Term* term)
    {
        return term->name;
    }

    void formatSource(RichSource* source, Term* term)
    {
        append_phrase(source, term->name, term, phrase_type::UNKNOWN_IDENTIFIER);
    }

    void setup(Branch& kernel)
    {
        UNKNOWN_IDENTIFIER_FUNC = import_function(kernel, evaluate, "unknown_identifier() -> any");
        function_t::get_attrs(UNKNOWN_IDENTIFIER_FUNC).toSource = toSourceString;
        function_t::get_attrs(UNKNOWN_IDENTIFIER_FUNC).formatSource = formatSource;
    }
}
}
