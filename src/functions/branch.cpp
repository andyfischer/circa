// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace branch_function {

    CA_FUNCTION(branch_evaluate)
    {
#ifndef BYTECODE
        evaluate_branch(CONTEXT, CALLER->nestedContents);
#endif
    }

    void writeBytecode(bytecode::WriteContext* context, Term* term)
    {
        Branch& branch = term->nestedContents;
        int stateContainer = -1;
        if (has_any_inlined_state(branch)) {
            // TODO: Fix
            stateContainer = context->nextStackIndex++;
        }
        bytecode::write_bytecode_for_branch(context, branch, stateContainer);
    }

    void format_source(StyledSource* source, Term* term)
    {
        if (term->boolPropOptional("syntax:literal-list", false)) {
            format_name_binding(source, term);
            append_phrase(source, "[", term, token::LBRACKET);
            Branch& contents = term->nestedContents;

            for (int i=0; i < contents.length(); i++) {
                Term* term = contents[i];
                format_term_source(source, term);
            }

            append_phrase(source, "]", term, token::RBRACKET);
        } else if (term->type == NAMESPACE_TYPE) {
            append_phrase(source, "namespace ", term, phrase_type::KEYWORD);
            append_phrase(source, term->name, term, phrase_type::TERM_NAME);
            append_phrase(source, term->stringPropOptional("syntax:postHeadingWs", "\n"),
                    term, token::WHITESPACE);
            format_branch_source(source, term->nestedContents, NULL);
            append_phrase(source, term->stringPropOptional("syntax:preEndWs", ""),
                    term, token::WHITESPACE);
                    
            append_phrase(source, "end", term, phrase_type::KEYWORD);
            
        } else {
            format_name_binding(source, term);
            format_branch_source(source, term->nestedContents, term);
        }
    }

    void setup(Branch& kernel)
    {
        BRANCH_FUNC = import_function(kernel, branch_evaluate, "branch()");
        function_t::get_attrs(BRANCH_FUNC).formatSource = format_source;
        function_t::get_attrs(BRANCH_FUNC).writeBytecode = writeBytecode;
    }
}
}
