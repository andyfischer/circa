// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace declared_state_function {

    void get_declared_state(caStack* stack)
    {
        caValue* value = circa_input(stack, 0);
        caValue* output = circa_output(stack, 0);
        Term* caller = (Term*) circa_caller_term(stack);

        // Try to cast 'value' to the declared type.
        if (value != NULL && !is_null(value)) {
            copy(value, output);
            bool cast_success = cast(output, declared_type(caller));

            // If this cast succeeded then we're done. If it failed then continue on
            // to use a default value.
            if (cast_success)
                return;
        }

        // We couldn't use the input value. If there is an initializer, then push that
        // to the stack.
        Branch* initializer = NULL;
        if (caller->input(1) != NULL)
            initializer = caller->input(1)->nestedContents;

        if (initializer != NULL) {
            // Remove the frame made for this call
            pop_frame(stack);

            // Call the initializer instead
            push_frame(stack, initializer);
            return;
        }

        // Otherwise, reset to the type's default value
        if (declared_type(caller) == &ANY_T)
            set_null(output);
        else
            make(declared_type(caller), output);
    }

    void formatSource(caValue* source, Term* term)
    {
        append_phrase(source, "state ", term, tok_State);

        if (term->hasProperty("syntax:explicitType")) {
            append_phrase(source, term->stringProp("syntax:explicitType",""),
                    term, name_TypeName);
            append_phrase(source, " ", term, tok_Whitespace);
        }

        append_phrase(source, term->name.c_str(), term, name_TermName);

        Term* defaultValue = NULL;
        Branch* initializer = NULL;

        if (term->input(1) != NULL)
            initializer = term->input(1)->nestedContents;

        if (initializer != NULL) {
            defaultValue = initializer->getFromEnd(0)->input(0);
            if (defaultValue->boolProp("hidden", false))
                defaultValue = defaultValue->input(0);
        }

        if (defaultValue != NULL) {
            append_phrase(source, " = ", term, name_None);
            if (defaultValue->name != "")
                append_phrase(source, get_relative_name_at(term, defaultValue),
                        term, name_TermName);
            else
                format_term_source(source, defaultValue);
        }
    }

    void setup(Branch* kernel)
    {
        FUNCS.declared_state = import_function(kernel, get_declared_state,
            "declared_state(state any value, any initializer :optional) -> any");
        as_function(FUNCS.declared_state)->formatSource = formatSource;
    }
}
}
