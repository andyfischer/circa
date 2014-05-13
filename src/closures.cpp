// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "closures.h"
#include "code_iterators.h"
#include "hashtable.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
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
    Block* compilationUnit = find_nearest_compilation_unit(block);

    for (BlockIterator it(block); it.unfinished(); it.advance()) {
        Term* innerTerm = *it;

        // Skip nonlocal() terms in this block. But, don't skip nonlocal terms that are
        // in nested major blocks.
        if (innerTerm->function == FUNCS.nonlocal
                && innerTerm->owningBlock == block)
            continue;

        for (int inputIndex=0; inputIndex < innerTerm->numInputs(); inputIndex++) {
            Term* input = innerTerm->input(inputIndex);
            if (input == NULL)
                continue;

            // No nonlocal needed for a pure value.
            if (is_value(input))
                continue;

            // No nonlocal needed if input is inside this major block.
            if (term_is_nested_in_block(input, block))
                continue;

            // No nonlocal needed if input is in a different compilation unit.
            // (Should only happen for a value reference to builtins)
            if (find_nearest_compilation_unit(input->owningBlock) != compilationUnit)
                continue;

            // This input needs a nonlocal() term.

            // Check if we've already created an input for this one
            Term* existing = find_nonlocal_term_for_input(block, input);

            if (existing != NULL) {
                remap_pointers_quick(innerTerm, input, existing);

            } else {
                // Create a new nonlocal term.
                Term* unbound = apply(block, FUNCS.nonlocal, TermList(input), &input->nameValue);
                set_declared_type(unbound, input->type);
                block->move(unbound, nextInsertPosition++);
                remap_pointers_quick(innerTerm, input, unbound);
                it.advance();
            }
        }
    }
}

void closure_block_evaluate(caStack* stack)
{
    Term* term = circa_caller_term(stack);
    caValue* closureOutput = circa_set_default_output(stack, 0);
    Block* block = nested_contents(term);
    set_block(list_get(closureOutput, 0), block);
    closure_save_all_bindings(closureOutput, stack);
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

Block* func_block(caValue* value)
{
    return as_block(list_get(value, 0));
}

caValue* func_bindings(caValue* value)
{
    return list_get(value, 1);
}

void closure_save_bindings_for_frame(caValue* closure, Frame* frame)
{
    Block* closureBlock = func_block(closure);

    if (closureBlock == NULL)
        return;

    Block* finishingBlock = frame->block;

    for (int i=0; i < closureBlock->length(); i++) {
        Term* term = closureBlock->get(i);
        if (term->function != FUNCS.nonlocal)
            continue;

        Term* input = term->input(0);

        if (input->owningBlock != finishingBlock)
            continue;

        // Capture this binding.

        Value key;
        set_term_ref(&key, input);
        caValue* value = frame_register(frame, input);

        // Don't overwrite an existing binding.
        if (hashtable_get(func_bindings(closure), &key) != NULL)
            continue;

        touch(closure);
        copy(value, hashtable_insert(func_bindings(closure), &key));
    }
}

void closure_save_all_bindings(caValue* closure, Stack* stack)
{
    Block* closureBlock = func_block(closure);

    if (closureBlock == NULL)
        return;

    for (int i=0; i < closureBlock->length(); i++) {
        Term* term = closureBlock->get(i);
        if (term->function != FUNCS.nonlocal)
            continue;

        Term* input = term->input(0);

        Value key;
        set_term_ref(&key, input);
        caValue* value = stack_find_nonlocal(top_frame(stack), input);

        // Don't overwrite an existing binding.
        if (hashtable_get(func_bindings(closure), &key) != NULL)
            continue;

        touch(closure);
        copy(value, hashtable_insert(func_bindings(closure), &key));
    }
}

void closures_install_functions(Block* kernel)
{
    install_function(kernel, "closure_block", closure_block_evaluate);
    install_function(kernel, "function_decl", closure_block_evaluate);

    FUNCS.func_apply = kernel->get("Func.apply");
}

} // namespace circa
