// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace unknown_identifier_function {

    CA_FUNCTION(evaluate)
    {
    }

    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, term->name, term, name_UnknownIdentifier);
    }

    void setup(Block* kernel)
    {
        FUNCS.unknown_identifier = import_function(kernel, evaluate, "unknown_identifier() -> any");
        as_function(FUNCS.unknown_identifier)->formatSource = formatSource;
    }
}
}
