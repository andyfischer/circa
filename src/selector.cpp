// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "kernel.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "selector.h"
#include "source_repro.h"
#include "string_type.h"

namespace circa {

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

bool is_accessor_function(Term* accessor)
{
    if (accessor->function == FUNCS.get_index
            || accessor->function == FUNCS.get_field)
        return true;

    if (accessor->function == FUNCS.dynamic_method)
        return true;

    // Future: We should be able to detect if a method behaves as an accessor, without
    // an explicit property.
    if (accessor->function->boolProp("fieldAccessor", false))
        return true;
    
    return false;
}

bool term_is_accessor_traceable(Term* accessor)
{
    if (!has_empty_name(accessor))
        return false;

    if (accessor->function == FUNCS.get_index
            || accessor->function == FUNCS.get_field
            || is_copying_call(accessor)
            || accessor->function == FUNCS.dynamic_method
            || is_subroutine(accessor->function))
        return true;

    return false;
}

void trace_accessor_chain(Term* accessor, TermList* chainResult)
{
    chainResult->resize(0);

    while (true) {
        chainResult->append(accessor);

        if (!term_is_accessor_traceable(accessor))
            break;

        // Continue the trace upward.
        accessor = accessor->input(0);
    }

    chainResult->reverse();
}

Term* find_accessor_head_term(Term* accessor)
{
    TermList chain;
    trace_accessor_chain(accessor, &chain);

    if (chain.length() == 0)
        return NULL;

    return chain[0];
}

Term* write_selector_for_accessor_chain(Block* block, TermList* chain)
{
    TermList selectorInputs;

    // Skip index 0 - this is the head term.
    
    for (int i=1; i < chain->length(); i++) {
        Term* term = chain->get(i);

        if (term->function == FUNCS.get_index
                || term->function == FUNCS.get_field) {

            selectorInputs.append(term->input(1));

        } else if (is_accessor_function(term)) {
            Term* element = create_string(block, term->stringProp("syntax:functionName", ""));
            selectorInputs.append(element);
        }
    }

    return apply(block, FUNCS.selector, selectorInputs);
}

Term* rebind_possible_accessor(Block* block, Term* accessor, Term* result)
{
    // Check if this isn't a recognized accessor.
    if (!has_empty_name(accessor)) {
        // Just create a named copy of 'result'.
        return apply(block, FUNCS.copy, TermList(result), accessor->nameSymbol);
    }

    TermList accessorChain;
    trace_accessor_chain(accessor, &accessorChain);

    Term* head = accessorChain[0];

    // Create the selector
    Term* selector = write_selector_for_accessor_chain(block, &accessorChain);

    Term* set = apply(block, FUNCS.set_with_selector,
            TermList(head, selector, result), head->nameSymbol);

    change_declared_type(set, declared_type(head));
    return set;
}

void resolve_rebind_operators_in_inputs(Block* block, Term* result)
{
    for (int inputIndex=0; inputIndex < result->numInputs(); inputIndex++) {
        Term* input = result->input(inputIndex);

        if (input == NULL)
            continue;

        // Walk upwards on 'input', see if one of the terms uses the @ operator.
        Term* head = input;
        Term* termBeforeHead = result;
        while (head->input(0) != NULL && term_is_accessor_traceable(head)) {
            termBeforeHead = head;
            head = head->input(0);
        }

        // Ignore term if there isn't a rebind.
        int inputIndexOfInterest = 0;
        if (termBeforeHead == result)
            inputIndexOfInterest = inputIndex;

        caValue* identifierRebindHint = term_get_input_property(termBeforeHead,
                inputIndexOfInterest, "syntax:identifierRebind");
        if (head == NULL || has_empty_name(head) || identifierRebindHint == NULL || !as_bool(identifierRebindHint))
            continue;

        if (input == head) {
            // No accessor expression, then just do a name rebind.
            rename(result, head->nameSymbol);
            result->setBoolProp("syntax:implicitName", true);
        } else {
            // Create a set_with_selector expression.
            TermList accessorChain;
            trace_accessor_chain(input, &accessorChain);

            Term* selector = write_selector_for_accessor_chain(block, &accessorChain);

            Term* set = apply(block, FUNCS.set_with_selector,
                    TermList(head, selector, result), head->nameSymbol);

            change_declared_type(set, declared_type(head));
        }
    }
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

void get_with_selector_evaluate(caStack* stack)
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
        // Unusual case; bail out with default formatting.
        format_term_source_default_formatting(source, term);
        return;
    }

    format_name_binding(source, term);
    format_source_for_input(source, term, 0, "", "");
    selector_format_source(source, selector);
}

void set_with_selector_evaluate(caStack* stack)
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

void set_with_selector__formatSource(caValue* source, Term* term)
{
    Term* selector = term->input(1);
    if (selector->function != FUNCS.selector) {
        format_term_source_default_formatting(source, term);
        return;
    }

    // Don't call format_name_binding here

    format_source_for_input(source, term, 0, "", "");

    selector_format_source(source, selector);

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

void selector_setup_funcs(Block* kernel)
{
    FUNCS.selector = 
        import_function(kernel, evaluate_selector, "selector(any elements :multiple) -> Selector");

    FUNCS.get_with_selector = 
        import_function(kernel, get_with_selector_evaluate,
            "get_with_selector(any object, Selector selector) -> any");
    as_function(FUNCS.get_with_selector)->formatSource = get_with_selector__formatSource;

    FUNCS.set_with_selector =
        import_function(kernel, set_with_selector_evaluate,
            "set_with_selector(any object, Selector selector, any newValue) -> any");
    as_function(FUNCS.set_with_selector)->formatSource = set_with_selector__formatSource;
}

} // namespace circa
