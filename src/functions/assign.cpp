// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {

namespace assign_function {

    void onCreateCall(Term* term)
    {
        write_setter_chain_for_assign_term(term);
    }

    Type* specializeType(Term* term)
    {
        Branch* contents = nested_contents(term);
        if (contents->length() > 0)
            return contents->get(contents->length()-1)->type;
        else
            return &ANY_T;
    }

    void formatSource(caValue* source, Term* term)
    {
        format_source_for_input(source, term, 0, "", "");

        Term* rhs = term->input(1);

        if (term->hasProperty("syntax:rebindOperator")) {
            append_phrase(source, rhs->stringPropOptional("syntax:functionName", ""),
                rhs, name_None);
            format_source_for_input(source, rhs, 1, "", "");
        } else {
            append_phrase(source, "=", term, name_None);
            format_source_for_input(source, term, 1, "", "");
        }
    }



    void setup(Branch* kernel)
    {
        FUNCS.assign = import_function(kernel, NULL, "assign(any, any) -> any");
        as_function(FUNCS.assign)->formatSource = formatSource;
        as_function(FUNCS.assign)->specializeType = specializeType;
        as_function(FUNCS.assign)->onCreateCall = onCreateCall;
    }
}
}
