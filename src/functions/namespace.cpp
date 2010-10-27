// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace namespace_function {

    CA_FUNCTION(evaluate)
    {
        Branch& contents = CALLER->nestedContents;
        push_stack_frame(STACK, contents.registerCount);
        evaluate_branch_existing_frame(CONTEXT, contents);

        List* stack = get_stack_frame(STACK, 0);
        TaggedValue outputTv;
        Dict* out = make_dict(&outputTv);
        out->clear();

        for (int i=0; i < contents.length(); i++) {
            Term* term = contents[i];
            if (term->name != "" && term->registerIndex != -1)
                swap(stack->get(term->registerIndex), out->insert(term->name.c_str()));
        }
        pop_stack_frame(STACK);
        swap(&outputTv, OUTPUT);
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
        Dict* dict = Dict::checkCast(INPUT(0));
        const char* name = STRING_INPUT(1);

        copy(dict->get(name), OUTPUT);
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
