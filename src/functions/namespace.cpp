// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace namespace_function {

    void evaluate(caStack* stack)
    {
        Term* caller = (Term*) circa_caller_term(stack);
        stack_push(stack, nested_contents(caller));
    }

    void format_source(caValue* source, Term* term)
    {
        append_phrase(source, "namespace ", term, sym_Keyword);
        append_phrase(source, term->name, term, sym_TermName);
        format_block_source(source, nested_contents(term), term);
        append_phrase(source, term->stringProp("syntax:preEndWs", ""),
                term, tok_Whitespace);
    }

    void early_setup(Block* kernel)
    {
        FUNCS.namespace_func = import_function(kernel, evaluate, "namespace()");
        block_set_format_source_func(function_contents(FUNCS.namespace_func), format_source);
        block_set_function_has_nested(function_contents(FUNCS.namespace_func), true);
    }
    void setup(Block* kernel) {}
}
}
