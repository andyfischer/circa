// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "hashtable.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "source_repro.h"
#include "tagged_value.h"

namespace circa {

Term* find_nonlocal_term_for_input(Block* block, Term* input)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->function == FUNCS.nonlocal && term->input(0) == input)
            return term;
    }
    return NULL;
}

void insert_nonlocal_terms(Block* block)
{
    int nextInsertPosition = count_input_placeholders(block);

    for (BlockIterator it(block); it.unfinished(); it.advance()) {
        Term* innerTerm = *it;
        for (int inputIndex=0; inputIndex < innerTerm->numInputs(); inputIndex++) {
            Term* input = innerTerm->input(inputIndex);
            if (input == NULL)
                continue;

            if (term_is_nested_in_block(input, block))
                continue;

            // This is a nonlocal reference

            // Check if we've already created an input for this one
            Term* existing = find_nonlocal_term_for_input(block, input);

            if (existing != NULL) {
                remap_pointers_quick(innerTerm, input, existing);

            } else {
                // Create a new nonlocal term.
                Term* unbound = apply(block, FUNCS.nonlocal,
                    TermList(input), &input->nameValue);
                change_declared_type(unbound, input->type);
                block->move(unbound, nextInsertPosition++);
                remap_pointers_quick(innerTerm, input, unbound);
                it.advance();
            }
        }
    }
}

void closure_block_formatSource(caValue* source, Term* term)
{
    format_name_binding(source, term);
    format_block_source(source, nested_contents(term), term);
}

void closure_block_evaluate(caStack* stack)
{
    Term* term = circa_caller_term(stack);
    caValue* closureOutput = circa_create_default_output(stack, 0);
    Block* block = nested_contents(term);
    set_block(list_get(closureOutput, 0), block);

#if 0
    caValue* bindings = list_get(closureOutput, 1);
    set_list(bindings, 0);

    // Capture unbound inputs.
    for (int i=count_input_placeholders(block); i < block->length(); i++) {
        Term* unbound = block->get(i);
        if (unbound->function != FUNCS.unbound_input)
            break;

        caValue* input = stack_find_active_value(stack_top_parent(stack), unbound->input(0));
        if (input == NULL)
            set_null(list_append(bindings));
        else
            copy(input, list_append(bindings));
    }
#endif
}

void add_bindings_to_closure_output(Stack* stack, caValue* closure)
{
    Block* closureBlock = as_block(list_get(closure, 0));
    Frame* top = stack_top(stack);
    Block* finishingBlock = top->block;

    for (int i=0; i < closureBlock->length(); i++) {
        Term* term = closureBlock->get(i);
        for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
            Term* input = term->input(inputIndex);
            if (input == NULL)
                continue;

            if (input->owningBlock == finishingBlock) {
                // Capture this binding.

                Value key;
                set_term_ref(&key, input);
                caValue* value = frame_register(top, input);
                touch(closure);

                copy(value, hashtable_insert(list_get(closure, 1), &key));
            }
        }
    }
}

bool is_closure(caValue* value)
{
    return value->value_type == TYPES.func;
}

caValue* closure_get_block(caValue* value)
{
    ca_assert(is_closure(value));
    return list_get(value, 0);
}

caValue* closure_get_bindings(caValue* value)
{
    ca_assert(is_closure(value));
    return list_get(value, 1);
}

void set_closure(caValue* value, Block* block, caValue* bindings)
{
    make(TYPES.func, value);
    touch(value);
    set_block(closure_get_block(value), block);
    if (bindings != NULL)
        set_value(closure_get_bindings(value), bindings);
}

void closures_install_functions(Block* kernel)
{
    FUNCS.closure_block = install_function(kernel, "closure_block", closure_block_evaluate);
    block_set_format_source_func(function_contents(FUNCS.closure_block), closure_block_formatSource);

    FUNCS.function_decl = install_function(kernel, "function_decl", closure_block_evaluate);

#if 0
    FUNCS.unbound_input = install_function(kernel, "unbound_input", NULL);
    block_set_evaluation_empty(function_contents(FUNCS.unbound_input), true);
#endif

    FUNCS.func_apply = kernel->get("Func.apply");
}

} // namespace circa
