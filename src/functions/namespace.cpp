// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace namespace_function {

    CA_FUNCTION(evaluate)
    {
        Branch& contents = CALLER->nestedContents;
        evaluate_branch_internal(CONTEXT, contents);
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

    CA_FUNCTION(get_namespace_field)
    {
        Branch& namespaceContents = INPUT_TERM(0)->nestedContents;
        const char* name = STRING_INPUT(1);

        Term* term = namespaceContents[name];

        if (term == NULL)
            return error_occurred(CONTEXT, CALLER, "couldn't find name");

        copy(get_local(term), OUTPUT);
    }

    Term* get_namespace_field_specialize_type(Term* caller)
    {
        Term* type = parser::statically_resolve_namespace_access(caller)->type;
        if (type != NULL) return type;
        return ANY_TYPE;
    }

    void get_namespace_field_format_source(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        format_source_for_input(source, term, 0);
        append_phrase(source, ":", term, token::COLON);
        append_phrase(source, term->input(1)->asString(), term, phrase_type::TERM_NAME);
    }

    void early_setup(Branch& kernel)
    {
        NAMESPACE_FUNC = import_function(kernel, evaluate, "namespace() -> Dict");
        function_t::get_attrs(NAMESPACE_FUNC).formatSource = format_source;

        GET_NAMESPACE_FIELD = import_function(kernel, get_namespace_field,
                "get_namespace_field(Dict, string) -> any");
        function_t::get_attrs(GET_NAMESPACE_FIELD).formatSource =
            get_namespace_field_format_source;
        function_t::get_attrs(GET_NAMESPACE_FIELD).specializeType =
            get_namespace_field_specialize_type;

    }
    void setup(Branch& kernel) {}
}
}
