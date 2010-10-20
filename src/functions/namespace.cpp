// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace namespace_function {

    CA_FUNCTION(evaluate)
    {
        Branch& contents = CALLER->nestedContents;
        evaluate_branch_existing_frame(CONTEXT, contents);
    }

    int get_register_count(Term* term)
    {
        Branch& contents = term->nestedContents;
        int count = 0;
        for (int i=0; i < contents.length(); i++)
            count += circa::get_register_count(contents[i]);
        return count;
    }

    void assign_registers(Term* term)
    {
        Branch& contents = term->nestedContents;
        int next = term->registerIndex;
        for (int i=0; i < contents.length(); i++) {
            Term* item = contents[i];
            int count = circa::get_register_count(item);
            if (count != 0)
                item->registerIndex = next;
            next += count;
        }
    }

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "namespace ", term, phrase_type::KEYWORD);
        append_phrase(source, term->name, term, phrase_type::TERM_NAME);
        append_phrase(source, term->stringPropOptional("syntax:postHeadingWs", "\n"),
                term, token::WHITESPACE);
        format_branch_source(source, term->nestedContents, NULL);
        append_phrase(source, term->stringPropOptional("syntax:preEndWs", ""),
                term, token::WHITESPACE);
                
        append_phrase(source, "end", term, phrase_type::KEYWORD);
    }

    void early_setup(Branch& kernel)
    {
        NAMESPACE_FUNC = import_function(kernel, evaluate, "namespace()");
        function_t::get_attrs(NAMESPACE_FUNC).formatSource = format_source;
        function_t::get_attrs(NAMESPACE_FUNC).getRegisterCount = get_register_count;
        function_t::get_attrs(NAMESPACE_FUNC).assignRegisters = assign_registers;
    }
    void setup(Branch& kernel) {}
}
}
