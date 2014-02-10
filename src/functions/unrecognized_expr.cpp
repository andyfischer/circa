// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace unrecognized_expr_function {

    void syntax_error(caStack* stack)
    {
        Value msg;
        set_string(&msg, "");
        string_append(&msg, circa_caller_term(stack)->stringProp(sym_Message,"Syntax error").c_str());
        circa_output_error(stack, as_cstring(&msg));
    }

    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, term->stringProp(sym_OriginalText,""), term, sym_None);
    }

    void setup(Block* kernel)
    {
        FUNCS.syntax_error = import_function(kernel, syntax_error, "syntax_error(i :multiple)");
        block_set_format_source_func(function_contents(FUNCS.syntax_error), formatSource);
    }
}
}
