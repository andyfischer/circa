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
        caValue* field = get_field(value, as_cstring(selectorElement));
        if (field == NULL) {
            set_error_string(error, "Field not found: ");
            string_append(error, selectorElement);
            return NULL;
        }
        return field;
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
    Term* bottomAccessor = accessor;

    while (true) {
        // Stop when we find a named term, unless it's the first term.
        if (accessor != bottomAccessor && !has_empty_name(accessor))
            return accessor;

        if (accessor->function == FUNCS.get_index
                || accessor->function == FUNCS.get_field
                || is_copying_call(accessor)
                || accessor->function == FUNCS.dynamic_method
                || is_subroutine(accessor->function)) {

            // Continue the trace upward.
            accessor = accessor->input(0);
            continue;
        }

        // Accessor search can't continue past this term.
        return accessor;
    }
}

void trace_selector_from_accessor(Term* head, Term* accessor, caValue* selectorOut)
{
    set_list(selectorOut, 0);

    Term* currentTerm = accessor;

    while (true) {
        // Stop when we reach the head term
        if (accessor == head)
            break;

        if (accessor->function == FUNCS.get_index
                || accessor->function == FUNCS.get_field) {

            // Accessor is get_index or get_field. Append the value to our selector
            // (as long as it's a plain value)
            
            Term* indexTerm = accessor->input(1);
            // TODO: Need to handle non-value indices
            if (!is_value(indexTerm))
                break;

            copy(term_value(indexTerm), list_append(selectorOut));
            accessor = accessor->input(0);
            continue;
        } else if (is_copying_call(accessor)) {
            accessor = accessor->input(0);

        } else if (accessor->function == FUNCS.dynamic_method) {
            // Dynamic method: do a string lookup using function name.
            copy(term_get_property(accessor, "syntax:functionName"),
                    list_append(selectorOut));
            accessor = accessor->input(0);

        } else if (is_subroutine(accessor->function)) {
            // Recursively look in this function.
            Branch* branch = function_contents(accessor->function);
            Term* branchOutput = get_output_placeholder(branch, 0);
            Term* nestedHead = find_accessor_head_term(branchOutput);

            if (!is_input_placeholder(nestedHead)) {
                // The nested trace did not reach the subroutine's input, so the subroutine
                // isn't an accessor that we understand. Stop here.
                break;
            }

            // Trace did reach subroutine's input. Append this selector section, and continue
            // searching from the function's input.
            circa::Value nestedSelector;
            trace_selector_from_accessor(nestedHead, branchOutput, &nestedSelector);
            list_extend(selectorOut, &nestedSelector);

            accessor = accessor->input(input_placeholder_index(nestedHead));

            if (accessor == NULL)
                break;
        } else {
            // This term isn't recognized as an accessor.
            break;
        }
    }

    // Selector list was built from the bottom-up, reverse so that it's top-down.
    list_reverse(selectorOut);
}

Term* rebind_possible_accessor(Branch* branch, Term* accessor, Term* result)
{
    circa::Value selector;

    // Check if this isn't a recognized accessor.
    if (!has_empty_name(accessor)) {
        // Just create a named copy of 'result'.
        return apply(branch, FUNCS.copy, TermList(result), accessor->nameSymbol);
    }

    Term* head = find_accessor_head_term(accessor);

    Term* set = apply(branch, FUNCS.assign_to_element,
            TermList(head, accessor, result), head->nameSymbol);

    change_declared_type(set, declared_type(head));
    return set;
}

void evaluate_selector_reflect(caStack* stack)
{
    Term* accessor = ((Term*) circa_caller_term(stack))->input(0);

    Term* head = find_accessor_head_term(accessor);

    trace_selector_from_accessor(head, accessor, circa_output(stack, 0));
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

void evaluate_assign_to_element(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);
    
    caValue* newValue = circa_input(stack, 2);

    Term* caller = (Term*) circa_caller_term(stack);
    Term* head = caller->input(0);
    Term* accessor = caller->input(1);

    // Trace the selector on the fly. TODO: Should precache this.
    circa::Value selector;
    trace_selector_from_accessor(head, accessor, &selector);

    circa::Value error;

    set_with_selector(out, &selector, newValue, &error);

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
void assign_to_element__formatSource(caValue* source, Term* term)
{
    // Don't call format_name_binding here

    // Left-hand-side
    format_source_for_input(source, term, 1, "", "");

    append_phrase(source, term->stringProp("syntax:preEqualsSpace",""), term, tok_Whitespace);

    if (term->hasProperty("syntax:rebindOperator")) {
        append_phrase(source, term->stringProp("syntax:rebindOperator",""), term, tok_Equals);
        append_phrase(source, term->stringProp("syntax:postEqualsSpace",""), term, tok_Whitespace);
        format_source_for_input(source, term->input(2), 1, "", "");
    } else {
        append_phrase(source, "=", term, tok_Equals);
        append_phrase(source, term->stringProp("syntax:postEqualsSpace",""), term, tok_Whitespace);
        format_source_for_input(source, term, 2, "", "");
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

    FUNCS.assign_to_element =
        import_function(kernel, evaluate_assign_to_element,
            "assign_to_element(any object, any lhs :meta, any newValue) -> any");
    as_function(FUNCS.assign_to_element)->formatSource = assign_to_element__formatSource;
}

} // namespace circa
