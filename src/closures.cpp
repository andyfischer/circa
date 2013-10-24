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

        // If we find a nonlocal() term that is in a different major block, then
        // we do need to create a nonlocal() in this major block. But if the
        // nonlocal() is in the same major block, ignore it.
        if (innerTerm->function == FUNCS.nonlocal
                && find_nearest_major_block(innerTerm->owningBlock) == block)
            continue;

        for (int inputIndex=0; inputIndex < innerTerm->numInputs(); inputIndex++) {
            Term* input = innerTerm->input(inputIndex);
            if (input == NULL)
                continue;

            if (is_value(input))
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
        caValue* value = stack_find_active_value(stack_top(stack), input);

        // Don't overwrite an existing binding.
        if (hashtable_get(func_bindings(closure), &key) != NULL)
            continue;

        touch(closure);
        copy(value, hashtable_insert(func_bindings(closure), &key));
    }
}

void Func__freeze(caStack* stack)
{
    caValue* self = circa_input(stack, 0);
    caValue* out = circa_output(stack, 0);

    copy(self, out);

    closure_save_all_bindings(out, stack);
}

void closures_install_functions(Block* kernel)
{
    install_function(kernel, "closure_block", closure_block_evaluate);
    block_set_format_source_func(function_contents(FUNCS.closure_block), closure_block_formatSource);

    install_function(kernel, "function_decl", closure_block_evaluate);

    FUNCS.func_apply = kernel->get("Func.apply");

    install_function(kernel, "Func.freeze", Func__freeze);
}

} // namespace circa
