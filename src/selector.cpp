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

caValue* selector_advance(caValue* value, caValue* selectorElement, caValue* error)
{
    if (is_int(selectorElement)) {
        int selectorIndex = as_int(selectorElement);

        if (!is_list(value)) {
            std::string msg;
            msg += "Value is not indexable: ";
            msg += to_string(value);
            set_error_string(error, msg.c_str());
            return NULL;
        }

        if (selectorIndex >= list_length(value)) {
            std::stringstream msg;
            msg << "Index ";
            msg << selectorIndex;
            msg << " is out of range";
            set_error_string(error, msg.str().c_str());
            return NULL;
        }

        return get_index(value, selectorIndex);
    }
    else if (is_string(selectorElement)) {
        return get_field(value, as_cstring(selectorElement));
    }
}

caValue* get_with_selector(caValue* root, caValue* selector, caValue* error)
{
    caValue* element = root;
    ca_assert(is_null(error));

    for (int i=0; i < list_length(selector); i++) {
        caValue* selectorElement = list_get(selector, i);
        element = selector_advance(element, selectorElement, error);

        if (!is_null(error))
            return NULL;
    }

    return element;
}

void set_with_selector(caValue* root, caValue* selector, caValue* newValue, caValue* error)
{
    caValue* element = root;
    ca_assert(is_null(error));

    for (int i=0; i < list_length(selector); i++) {
        touch(element);
        caValue* selectorElement = list_get(selector, i);
        element = selector_advance(element, selectorElement, error);

        if (!is_null(error))
            return;
    }

    copy(newValue, element);
}

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

void evaluate_set_with_selector(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);
    
    caValue* selector = circa_input(stack, 1);
    caValue* newValue = circa_input(stack, 2);

    circa::Value error;

    set_with_selector(out, selector, newValue, &error);

    if (!is_null(&error)) {
        copy(&error, circa_output(stack, 0));
        raise_error(stack);
        return;
    }
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
