// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "kernel.h"
#include "hashtable.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "selector.h"
#include "string_type.h"
#include "type.h"

namespace circa {

Value* selector_advance(Value* value, Value* selectorElement, Value* error)
{
    if (is_int(selectorElement)) {
        int selectorIndex = as_int(selectorElement);

        if (!is_list(value)) {
            if (error != NULL) {
                set_error_string(error, "Value is not indexable: ");
                string_append_quoted(error, value);
            }
            return NULL;
        }

        if (selectorIndex >= list_length(value)) {
            if (error != NULL) {
                set_error_string(error, "Index ");
                string_append(error, selectorIndex);
                string_append(error, " is out of range");
            }
            return NULL;
        }

        return get_index(value, selectorIndex);

    } else if (is_string(selectorElement)) {
        Value* field = get_field(value, selectorElement, NULL);
        if (field == NULL) {
            if (error != NULL) {
                set_error_string(error, "Field not found: ");
                string_append(error, selectorElement);
            }
            return NULL;
        }
        return field;
    } else {
        if (error != NULL) {
            set_error_string(error, "Unrecognized selector element: ");
            string_append_quoted(error, selectorElement);
        }
        return NULL;
    }
}

Type* element_type_from_selector(Type* type, Value* selectorElement)
{
    if (!is_struct_type(type))
        return TYPES.any;

    if (is_int(selectorElement)) {
        return compound_type_get_field_type(type, as_int(selectorElement));
    } else if (is_string(selectorElement)) {
        int index = list_find_field_index_by_name(type, as_cstring(selectorElement));
        return compound_type_get_field_type(type, index);
    }

    return TYPES.any;
}

Value* get_with_selector(Value* root, Value* selector, Value* error)
{
    Value* element = root;

    for (int i=0; i < list_length(selector); i++) {
        Value* selectorElement = list_get(selector, i);
        element = selector_advance(element, selectorElement, error);

        if (element == NULL)
            return NULL;
    }

    return element;
}

void set_with_selector(Value* value, Value* selector, Value* newValue, Value* error)
{
    if (list_empty(selector)) {
        copy(newValue, value);
        return;
    }

    for (int selectorIndex=0;; selectorIndex++) {

        if (touch_is_necessary(value) && is_list(value))
            stat_increment(SetWithSelector_Touch_List);
        if (touch_is_necessary(value) && is_hashtable(value))
            stat_increment(SetWithSelector_Touch_Hashtable);

        touch(value);
        Value* selectorElement = list_get(selector, selectorIndex);
        Value* element = selector_advance(value, selectorElement, error);

        if (element == NULL)
            return;

        if (selectorIndex+1 == list_length(selector)) {
            copy(newValue, element);
            Type* elementType = element_type_from_selector(value->value_type, selectorElement);
            if (!cast(element, elementType)) {
                if (error != NULL) {
                    set_string(error, "Couldn't cast value ");
                    string_append_quoted(error, newValue);
                    string_append(error, " to type ");
                    string_append(error, &elementType->name);
                    string_append(error, " (element ");
                    string_append_quoted(error, selectorElement);
                    string_append(error, " of type ");
                    string_append(error, &value->value_type->name);
                    string_append(error, ")");
                }
                return;
            }
            
            break;
        }

        value = element;
        // loop
    }
    return;
}

Value* path_touch_and_init_map(Value* value, Value* path)
{
    Value* currentElement = value;
    for (int i=0; i < list_length(path); i++) {
        touch(currentElement);

        if (!is_hashtable(currentElement))
            set_hashtable(currentElement);

        Value* nextElement = hashtable_insert(currentElement, path->index(i));

        currentElement = nextElement;
    }
    return currentElement;
}

Value* path_get(Value* value, Value* path)
{
    Value* currentElement = value;
    for (int i=0; i < list_length(path); i++) {
        if (!is_hashtable(currentElement))
            return NULL;

        Value* nextElement = hashtable_get(currentElement, path->index(i));

        if (nextElement == NULL)
            return NULL;

        currentElement = nextElement;
    }
    return currentElement;
}

void path_delete(Value* value, Value* path)
{
    Value* currentElement = value;
    for (int i=0; i < list_length(path); i++) {

        if (!is_hashtable(currentElement))
            return;

        touch(currentElement);

        if (i+1 >= list_length(path)) {
            hashtable_remove(currentElement, path->index(i));
            return;
        }

        Value* nextElement = hashtable_get(currentElement, path->index(i));

        if (nextElement == NULL)
            return;

        currentElement = nextElement;
    }
}

void evaluate_selector(Stack* stack)
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
    if (accessor->function->boolProp(sym_FieldAccessor, false))
        return true;
    
    return false;
}

bool term_is_accessor_traceable(Term* accessor)
{
    ca_assert(FUNCS.get_index != NULL);
    ca_assert(FUNCS.get_field != NULL);
    ca_assert(FUNCS.dynamic_method != NULL);

    if (!has_empty_name(accessor))
        return false;

    if (accessor->function == FUNCS.get_index
            || accessor->function == FUNCS.get_field
            || is_copying_call(accessor)
            || accessor->function == FUNCS.dynamic_method
            || accessor->function->boolProp(sym_FieldAccessor, false)
            || accessor->function->boolProp(sym_Setter, false))
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

        if (accessor->input(0) == NULL)
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
            Term* element = create_string(block, term->stringProp(sym_Syntax_FunctionName, "").c_str());
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
        return apply(block, FUNCS.copy, TermList(result), &accessor->nameValue);
    }

    TermList accessorChain;
    trace_accessor_chain(accessor, &accessorChain);

    Term* head = accessorChain[0];

    // Create the selector
    Term* selector = write_selector_for_accessor_chain(block, &accessorChain);

    Term* set = apply(block, FUNCS.set_with_selector,
            TermList(head, selector, result), &head->nameValue);

    set_declared_type(set, declared_type(head));
    return set;
}

Term* find_or_create_next_unnamed_term_output(Term* term)
{
    // require statement has a 'soft' name binding. It can receive a name from
    // the module name, but any applied name will replace this.
    if (term->function == FUNCS.require)
        return term;
    
    for (int i=0;; i++) {
        Term* output = get_output_term(term, i);
        if (output == NULL)
            return find_or_create_output_term(term, i);

        if (has_empty_name(output))
            return output;
    }

    internal_error("unreachable");
    return NULL;
}

Term* resolve_rebind_operators_in_inputs(Block* block, Term* term)
{
    for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
        Term* input = term->input(inputIndex);

        if (input == NULL)
            continue;

        // Walk upwards on 'input', see if one of the terms uses the @ operator.
        Term* head = input;
        Term* termBeforeHead = term;
        while (head->input(0) != NULL && term_is_accessor_traceable(head)) {
            termBeforeHead = head;
            head = head->input(0);
        }

        // Ignore term if there isn't a rebind.
        int inputIndexOfInterest = 0;
        if (termBeforeHead == term)
            inputIndexOfInterest = inputIndex;

        Value* identifierRebindHint = term_get_input_property(termBeforeHead,
                inputIndexOfInterest, sym_Syntax_IdentifierRebind);
        if (head == NULL || has_empty_name(head)
                || identifierRebindHint == NULL
                || !as_bool(identifierRebindHint))
            continue;

        // Find the output term (may be an extra_output or may be 'term')
        Term* output = find_or_create_next_unnamed_term_output(term);

        if (input == head) {
            // No accessor expression, then just do a name rebind.
            rename(output, &head->nameValue);
            output->setBoolProp(sym_Syntax_ImplicitName, true);
        } else {
            // Create a set_with_selector expression.
            TermList accessorChain;
            trace_accessor_chain(input, &accessorChain);

            Term* selector = write_selector_for_accessor_chain(block, &accessorChain);

            Term* set = apply(block, FUNCS.set_with_selector,
                    TermList(head, selector, output), &head->nameValue);

            set_declared_type(set, declared_type(head));
            return set;
        }
    }

    return NULL;
}

void get_with_selector_evaluate(Stack* stack)
{
    Value* root = circa_input(stack, 0);
    Value* selector = circa_input(stack, 1);

    circa::Value error;

    Value* result = get_with_selector(root, selector, &error);

    if (!is_null(&error)) {
        copy(&error, circa_output(stack, 0));
        raise_error(stack);
        return;
    }

    copy(result, circa_output(stack, 0));
}

void set_with_selector_evaluate(Stack* stack)
{
    Value* out = circa_output(stack, 0);
    move(circa_input(stack, 0), out);
    
    Value* selector = circa_input(stack, 1);
    Value* newValue = circa_input(stack, 2);

    circa::Value error;
    set_with_selector(out, selector, newValue, &error);

    if (!is_null(&error)) {
        circa_output_error_val(stack, &error);
        return;
    }
}

void path_get_func(Stack* stack)
{
    Value* result = path_get(circa_input(stack, 0), circa_input(stack, 1));
    if (result != NULL)
        copy(result, circa_output(stack, 0));
    else
        set_null(circa_output(stack, 0));
}

void path_set_func(Stack* stack)
{
    move(circa_input(stack, 0), circa_output(stack, 0));
    move(circa_input(stack, 2), path_touch_and_init_map(circa_output(stack, 0), circa_input(stack, 1)));
}

void path_delete_func(Stack* stack)
{
    move(circa_input(stack, 0), circa_output(stack, 0));
    path_delete(circa_output(stack, 0), circa_input(stack, 1));
}

void selector_setup_funcs(NativePatch* patch)
{
    circa_patch_function(patch, "selector", evaluate_selector);
    circa_patch_function(patch, "get_with_selector", get_with_selector_evaluate);
    circa_patch_function(patch, "set_with_selector", set_with_selector_evaluate);
    circa_patch_function(patch, "get", path_get_func);
    circa_patch_function(patch, "set", path_set_func);
    circa_patch_function(patch, "delete", path_delete_func);
}

} // namespace circa
