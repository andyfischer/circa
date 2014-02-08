// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace get_index_function {

    void hosted_get_index(caStack* stack)
    {
        caValue* list = circa_input(stack, 0);
        int index = circa_int_input(stack, 1);

        if (index < 0) {
            char indexStr[40];
            sprintf(indexStr, "Negative index: %d", index);
            return circa_output_error(stack, indexStr);
        } else if (index >= list_length(list)) {
            char indexStr[40];
            sprintf(indexStr, "Index out of range: %d", index);
            return circa_output_error(stack, indexStr);
        }

        caValue* result = get_index(list, index);

        copy(result, circa_output(stack, 0));
        cast(circa_output(stack, 0), declared_type((Term*) circa_caller_term(stack)));
    }
    Type* specializeType(Term* term)
    {
        return infer_type_of_get_index(term->input(0));
    }

    void format_source(caValue* source, Term* term)
    {
        if (term->boolProp(sym_Syntax_Brackets, false)) {
            format_name_binding(source, term);
            format_source_for_input(source, term, 0);
            append_phrase(source, "[", term, tok_LSquare);
            format_source_for_input(source, term, 1);
            append_phrase(source, "]", term, tok_RSquare);
        } else {
            format_term_source_default_formatting(source, term);
        }
    }

    void setup(Block* kernel)
    {
        FUNCS.get_index = import_function(kernel, hosted_get_index,
                "get_index(List list, int index) -> any");
        block_set_specialize_type_func(function_contents(FUNCS.get_index), specializeType);
        block_set_format_source_func(function_contents(FUNCS.get_index), format_source);
    }
}
}
