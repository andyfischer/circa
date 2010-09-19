// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"
#include "types/dict.h"

namespace circa {
namespace inline_state_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(get_top_level_state, "get_top_level_state() -> any")
    {
        copy(&CONTEXT->topLevelState, OUTPUT);
    }
    CA_DEFINE_FUNCTION(set_top_level_state, "set_top_level_state(any)")
    {
        copy(INPUT(0), &CONTEXT->topLevelState);
    }

    CA_DEFINE_FUNCTION(get_state_field,
            "get_state_field(any +optional, string name, any default_value +optional) -> any")
    {
        ca_assert(INPUT(1) != NULL);

        TaggedValue *container = INPUT(0);
        if (!is_dict(container)) make_dict(container);

        Dict* dict = Dict::checkCast(container);
        TaggedValue* value = dict->get(STRING_INPUT(1));
        if (value) {
            // todo: check if we need to cast this value
            copy(value, OUTPUT);

        // If we didn't find the value, see if they provided a default
        } else if (INPUT(2) != NULL) {
            copy(INPUT(2), OUTPUT);

        // Otherwise, reset to default value of type
        } else {
            ca_assert(CALLER != NULL);
            change_type(OUTPUT, type_contents(CALLER->type));
            reset(OUTPUT);
        }
    }

    void get_state_field_write_bytecode(bytecode::WriteContext* context, Term* term)
    {
        Term* nameTerm = term->input(1);
        int name = nameTerm->stackIndex;
        int defaultValue = term->input(2) == NULL ? -1 : term->input(2)->stackIndex;
        if (term->stackIndex == -1)
            term->stackIndex = context->nextStackIndex++;
        bytecode::write_get_state_field(context, term, name, defaultValue, term->stackIndex);

        context->appendStateFieldStore(as_string(nameTerm), name);
    }

    CA_DEFINE_FUNCTION(set_state_field,
            "set_state_field(any container, string name, any field) -> any")
    {
        copy(INPUT(0), OUTPUT);
        TaggedValue *container = OUTPUT;
        touch(container);
        if (!is_dict(container)) make_dict(container);
        Dict* dict = Dict::checkCast(container);
        dict->set(STRING_INPUT(1), INPUT(2));
        #if 0
        std::cout << "set_state_field("
            << INPUT(0)->toString() << ","
            << INPUT(1)->toString() << ","
            << INPUT(2)->toString() << ")" << std::endl;
        std::cout << "result = " << OUTPUT->toString() << std::endl;
        #endif
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "state ", term, token::STATE);

        if (term->hasProperty("syntax:explicitType")) {
            append_phrase(source, term->stringProp("syntax:explicitType"),
                    term, phrase_type::TYPE_NAME);
            append_phrase(source, " ", term, token::WHITESPACE);
        }

        append_phrase(source, term->name.c_str(), term, phrase_type::TERM_NAME);

        if (term->hasProperty("initializedBy")) {
            Term* initializedBy = term->refProp("initializedBy");
            append_phrase(source, " = ", term, phrase_type::UNDEFINED);
            if (initializedBy->name != "")
                append_phrase(source, get_relative_name(term, initializedBy),
                        term, phrase_type::TERM_NAME);
            else
                format_term_source(source, initializedBy);
        }
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        function_t::get_attrs(kernel["get_state_field"]).writeBytecode =
            get_state_field_write_bytecode;
    }
}
}
