// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace copy_function {

    void evaluate(caStack* stack)
    {
        copy(circa_input(stack, 0), circa_output(stack, 0));
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
        FUNCS.copy = import_function(kernel, evaluate, "copy(any val) -> any");
        block_set_specialize_type_func(as_function2(FUNCS.copy), specializeType);
        as_function(FUNCS.copy)->formatSource = formatSource;
    }
}
}
