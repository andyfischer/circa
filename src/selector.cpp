// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "evaluation.h"
#include "kernel.h"
#include "importing.h"
#include "list.h"
#include "selector.h"
#include "source_repro.h"

namespace circa {

void selector_prepend(caValue* selector, caValue* element)
{
    copy(element, list_insert(selector, 0));
}

caValue* get_with_selector(caValue* root, caValue* selector, caValue* error)
{
    caValue* element = root;

    for (int i=0; i < list_length(selector); i++) {

        caValue* selectorElement = list_get(selector, i);

        if (is_int(selectorElement)) {
            int selectorIndex = as_int(selectorElement);

            if (!is_list(element)) {
                std::string msg;
                msg += "Value is not indexable: ";
                msg += to_string(element);
                set_error_string(error, msg.c_str());
                return NULL;
            }

            if (selectorIndex >= list_length(element)) {
                std::stringstream msg;
                msg << "Index ";
                msg << selectorIndex;
                if (list_length(selector) > 1)
                    msg << " (in selector position " << i << ")";


                msg << " is out of range";
                set_error_string(error, msg.str().c_str());
                return NULL;
            }

            element = get_index(element, selectorIndex);
        }
        else if (is_string(selectorElement))
            element = get_field(element, as_cstring(selectorElement));
        else
            internal_error("Unrecognized selector element");
    }

    return element;
}

/*
void get_selector_from_getter_chain(Term* getter, caValue* selector)
{
    while (true) {
        // Don't write a setter for a getter that already has a name; if the term has
        // a name then it's not part of this lexpr.
        if (getter->name != "")
            return;

        circa::Value element
        if (getter->function == FUNCS.get_index) {
            selector_prepend(selector
            return apply(branch, FUNCS.set_index, TermList(getter->input(0),
                getter->input(1), desiredValue));
        } else if (getter->function == FUNCS.get_field) {
            return apply(branch, FUNCS.set_field, TermList(getter->input(0),
                getter->input(1), desiredValue));
        } else if (getter->function == FUNCS.dynamic_method) {
            Term* fieldName = create_string(branch, getter->stringProp("syntax:functionName", ""));
            return apply(branch, FUNCS.set_field, TermList(getter->input(0),
                fieldName, desiredValue));
        }

        getter = getter->input(0);
    }
}
*/

void evaluate_selector(caStack* stack)
{
    copy(circa_input(stack, 0), circa_output(stack, 0));
}

void evaluate_get_with_selector(caStack* stack)
{
    caValue* root = circa_input(stack, 0);
    caValue* selector = circa_input(stack, 1);

    circa::Value error;

    caValue* result = get_with_selector(root, selector, &error);

    if (!is_null(&error)) {
        copy(&error, circa_output(stack, 0));
        raise_error(stack);
        return;
    }

    copy(result, circa_output(stack, 0));
}

void get_with_selector__formatSource(caValue* source, Term* term)
{
    Term* selector = term->input(1);
    if (selector->function != FUNCS.selector) {
        format_term_source_default_formatting(source, term);
        return;
    }

    format_source_for_input(source, term, 0, "", "");

    // Append subscripts for each selector element
    for (int i=0; i < selector->numInputs(); i++) {
        append_phrase(source, "[", term, tok_LBracket);
        format_source_for_input(source, selector, i, "", "");
        append_phrase(source, "]", term, tok_LBracket);
    }
}

void selector_setup_funcs(Branch* kernel)
{
    FUNCS.selector = 
        import_function(kernel, evaluate_selector, "selector(any elements :multiple) -> Selector");
    FUNCS.get_with_selector = 
        import_function(kernel, evaluate_get_with_selector,
            "get_with_selector(any object, Selector selector) -> any");

    as_function(FUNCS.get_with_selector)->formatSource = get_with_selector__formatSource;
}

} // namespace circa
