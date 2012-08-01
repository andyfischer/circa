// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "evaluation.h"
#include "kernel.h"
#include "importing.h"
#include "inspection.h"
#include "list.h"
#include "selector.h"
#include "source_repro.h"
#include "string_type.h"

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
            set_error_string(error, "Value is not indexable: ");
            string_append_quoted(error, value);
            return NULL;
        }

        if (selectorIndex >= list_length(value)) {
            set_error_string(error, "Index ");
            string_append(error, selectorIndex);
            string_append(error, " is out of range");
            return NULL;
        }

        return get_index(value, selectorIndex);
    }
    else if (is_string(selectorElement)) {
        return get_field(value, as_cstring(selectorElement));
    } else {
        set_error_string(error, "Unrecognized selector element: ");
        string_append_quoted(error, selectorElement);
        return NULL;
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

Term* find_accessor_head_term(Term* accessor)
{
    if (!has_empty_name(accessor))
        return accessor;

    return find_accessor_head_term(accessor->input(0));
}

struct SelectorFromAccessorTrace
{
    Term* head;
    circa::Value selectors;
};

void trace_selector_from_accessor(SelectorFromAccessorTrace* result, Term* accessor)
{
    set_list(&result->selectors, 0);

    while (true) {
        // Stop when we find a named term, unless it's the first term.
        if (!has_empty_name(accessor) && list_length(&result->selectors) > 0)
            break;

        if (accessor->function == FUNCS.get_index
                || accessor->function == FUNCS.get_field) {

            // Accessor is get_index or get_field. Append the value to our selector
            // (as long as it's a plain value)
            
            Term* indexTerm = accessor->input(1);
            if (!is_value(indexTerm))
                break;

            copy(term_value(indexTerm), list_append(&result->selectors));
            accessor = accessor->input(0);
            continue;
        } else if (is_copying_call(accessor)) {
            accessor = accessor->input(0);

        } else if (is_subroutine(accessor->function)) {
            // Recursively look in this function.
            Branch* branch = function_contents(accessor->function);
            SelectorFromAccessorTrace nestedSearch;
            trace_selector_from_accessor(&nestedSearch, get_output_placeholder(branch, 0));

            if (!is_input_placeholder(nestedSearch.head)) {
                // The nested trace did not reach the subroutine's input, so the subroutine
                // isn't an accessor that we understand. Stop here.
                break;
            }

            // Trace did reach subroutine's input. Append this selector section, and continue
            // searching from the function's input.
            list_extend(&result->selectors, &nestedSearch.selectors);
            accessor = accessor->input(input_placeholder_index(nestedSearch.head));

            if (accessor == NULL)
                break;
        } else {
            // This term isn't recognized as an accessor.
            break;
        }
    }

    result->head = accessor;
}

Term* write_set_selector_result(Branch* branch, Term* accessorExpr, Term* result)
{
    SelectorFromAccessorTrace trace;
    trace_selector_from_accessor(&trace, accessorExpr);

    Term* head = trace.head;
    Term* selector = apply(branch, FUNCS.selector_reflect, TermList(accessorExpr));

    Term* set = apply(branch, FUNCS.set_with_selector,
            TermList(head, accessorExpr, selector, result));

    change_declared_type(set, declared_type(head));
    rename(set, head->nameSymbol);
    return set;
}

void evaluate_selector_reflect(caStack* stack)
{
    Term* accessor = ((Term*) circa_caller_term(stack))->input(0);
    SelectorFromAccessorTrace trace;

    trace_selector_from_accessor(&trace, accessor);
    copy(&trace.selectors, circa_output(stack, 0));
}

void selector_format_source(caValue* source, Term* term)
{
    // Append subscripts for each selector element
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);

        if (is_value(input) && is_string(term_value(input))) {
            append_phrase(source, ".", input, tok_Dot);
            append_phrase(source, as_string(term_value(input)),
                    input, tok_Identifier);

        } else {
            append_phrase(source, "[", term, tok_LBracket);
            format_source_for_input(source, term, i, "", "");
            append_phrase(source, "]", term, tok_LBracket);
        }
    }
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
    
    caValue* selector = circa_input(stack, 2);
    caValue* newValue = circa_input(stack, 3);

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

    format_name_binding(source, term);
    format_source_for_input(source, term, 0, "", "");
    selector_format_source(source, selector);

}
void set_with_selector__formatSource(caValue* source, Term* term)
{
    // Don't call format_name_binding here

    // Left-hand-side
    format_source_for_input(source, term, 1, "", "");

    append_phrase(source, term->stringProp("syntax:preEqualsSpace",""), term, tok_Whitespace);

    if (term->hasProperty("syntax:rebindOperator")) {
        append_phrase(source, term->stringProp("syntax:rebindOperator",""), term, tok_Equals);
        append_phrase(source, term->stringProp("syntax:postEqualsSpace",""), term, tok_Whitespace);
        format_source_for_input(source, term->input(3), 1, "", "");
    } else {
        append_phrase(source, "=", term, tok_Equals);
        append_phrase(source, term->stringProp("syntax:postEqualsSpace",""), term, tok_Whitespace);
        format_source_for_input(source, term, 3, "", "");
    }
}

void selector_setup_funcs(Branch* kernel)
{
    FUNCS.selector = 
        import_function(kernel, evaluate_selector, "selector(any elements :multiple) -> Selector");

    FUNCS.selector_reflect = import_function(kernel, evaluate_selector_reflect,
                "selector_reflect(any :meta) -> Selector");

    FUNCS.get_with_selector = 
        import_function(kernel, evaluate_get_with_selector,
            "get_with_selector(any object, Selector selector) -> any");
    as_function(FUNCS.get_with_selector)->formatSource = get_with_selector__formatSource;

    FUNCS.set_with_selector =
        import_function(kernel, evaluate_set_with_selector,
            "set_with_selector(any object, any lhs :meta, Selector selector, any value) -> any");
    as_function(FUNCS.set_with_selector)->formatSource = set_with_selector__formatSource;
}

} // namespace circa
