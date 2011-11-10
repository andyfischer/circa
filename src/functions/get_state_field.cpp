// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "types/dict.h"

namespace circa {
namespace get_state_field_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(get_state_field,
        "get_state_field(any container, any default_value :optional) -> any")
    {
        Dict* stateContainer = Dict::checkCast(INPUT(0));
        ca_assert(stateContainer != NULL);

        const char* name = CALLER->name.c_str();
        TaggedValue* value = stateContainer->get(name);

        // Try to cast 'value' to the declared type.
        if (value != NULL) {
            bool cast_success = cast(value, declared_type(CALLER), OUTPUT);

            // If this cast succeeded then we're done. If it failed then continue on
            // to use a default value.
            if (cast_success)
                return;
        }

        // Try to use initialValue from an input.
        if (INPUT_TERM(1) != NULL) {
            // Evaluate nested contents first, since the initial value might come from there.
            if (CALLER->nestedContents) 
                evaluate_branch_internal(CONTEXT, nested_contents(CALLER));
            
            bool cast_success = cast(INPUT(1), declared_type(CALLER), OUTPUT);

            if (!cast_success) {
                std::stringstream msg;
                msg << "Couldn't cast default value to type " <<
                    declared_type(CALLER)->name;
                ERROR_OCCURRED(msg.str().c_str());
            }
        } else {

            // Otherwise, reset to the type's default value
            create(declared_type(CALLER), OUTPUT);
            reset(OUTPUT);
        }
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

        Term* defaultValue = term->input(1);

        if (defaultValue == NULL && nested_contents(term)->length() > 0)
            defaultValue = nested_contents(term)->getFromEnd(0);

        if (defaultValue != NULL) {
            append_phrase(source, " = ", term, phrase_type::UNDEFINED);
            if (defaultValue->name != "")
                append_phrase(source, get_relative_name_at(term, defaultValue),
                        term, phrase_type::TERM_NAME);
            else
                format_term_source(source, defaultValue);
        }
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        GET_STATE_FIELD_FUNC = kernel->get("get_state_field");
        as_function(GET_STATE_FIELD_FUNC)->formatSource = formatSource;
    }
}
}
