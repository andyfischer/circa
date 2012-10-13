// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace namespace_function {

    void evaluate(caStack* stack)
    {
        Term* caller = (Term*) circa_caller_term(stack);
        push_frame(stack, nested_contents(caller));
    }

    void format_source(caValue* source, Term* term)
    {
        append_phrase(source, "namespace ", term, name_Keyword);
        append_phrase(source, term->name, term, name_TermName);
        format_branch_source(source, nested_contents(term), term);
        append_phrase(source, term->stringProp("syntax:preEndWs", ""),
                term, tok_Whitespace);
    }

    void early_setup(Branch* kernel)
    {
        FUNCS.namespace_func = import_function(kernel, evaluate, "namespace()");
        as_function(FUNCS.namespace_func)->formatSource = format_source;
    }
    void setup(Branch* kernel) {}
}
}
