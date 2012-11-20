// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace copy_function {

    CA_FUNCTION(evaluate)
    {
        CONSUME_INPUT(0, OUTPUT);
    }

    Type* specializeType(Term* caller)
    {
        return get_type_of_input(caller, 0);
    }

    void formatSource(caValue* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, get_relative_name_at(term, term->input(0)),
                term, tok_Identifier);
    }

    void setup(Block* kernel)
    {
        FUNCS.copy = import_function(kernel, evaluate, "copy(any) -> any");
        as_function(FUNCS.copy)->specializeType = specializeType;
        as_function(FUNCS.copy)->formatSource = formatSource;
    }
}
}
