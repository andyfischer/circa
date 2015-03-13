// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "closures.h"
#include "code_iterators.h"
#include "hashtable.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "tagged_value.h"
#include "term.h"

namespace circa {

Term* find_nonlocal_term_for_input(Block* block, Term* input)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->function == FUNCS.upvalue && term->input(0) == input)
            return term;
    }
    return NULL;
}

void insert_upvalue_terms(Block* block)
{
    if (get_parent_block(block) == global_builtins_block())
        return;

    int nextInsertPosition = count_input_placeholders(block);
    Block* compilationUnit = find_nearest_compilation_unit(block);

    for (BlockIterator it(block); it; ++it) {
        Term* innerTerm = *it;

        // Skip upvalue() terms in this block. But, don't skip upvalue terms that are
        // in nested major blocks.
        if (innerTerm->function == FUNCS.upvalue
                && innerTerm->owningBlock == block)
            continue;

        for (int depIndex=0; depIndex < innerTerm->numDependencies(); depIndex++) {
            Term* input = innerTerm->dependency(depIndex);
            if (input == NULL)
                continue;

            // No upvalue needed for a static value.
            if (has_static_value(input))
                continue;

            // Recursive call might not yet have its static value.
            if (block->owningTerm == input)
                continue;

            // No upvalue needed if input is inside this major block.
            if (term_is_nested_in_block(input, block))
                continue;

            // No upvalue needed if input is in a different compilation unit.
            // (Should only happen for a value reference to builtins)
            if (find_nearest_compilation_unit(input->owningBlock) != compilationUnit)
                continue;


            // This input needs a upvalue() term.

            // Check if we've already created an input for this one
            Term* existing = find_nonlocal_term_for_input(block, input);

            if (existing != NULL) {
                remap_pointers_quick(innerTerm, input, existing);

            } else {
                // Create a new upvalue term.
                Term* unbound = apply(block, FUNCS.upvalue, TermList(input), &input->nameValue);
                set_declared_type(unbound, input->type);
                block->move(unbound, nextInsertPosition++);
                remap_pointers_quick(innerTerm, input, unbound);
                it.advance();
            }
        }
    }
}

bool is_closure(Value* value)
{
    return value->value_type == TYPES.func;
}

// TODO: Delete in favor of func_block
Value* closure_get_block(Value* value)
{
    ca_assert(is_closure(value));
    return list_get(value, 0);
}

Value* closure_get_bindings(Value* value)
{
    ca_assert(is_closure(value));
    return list_get(value, 1);
}

void set_closure(Value* value, Block* block, Value* bindings)
{
    Value bindingsCopy;
    if (bindings != NULL)
        copy(bindings, &bindingsCopy);

    make(TYPES.func, value);
    touch(value);
    set_block(closure_get_block(value), block);
    move(&bindingsCopy, closure_get_bindings(value));
}

Block* func_block(Value* value)
{
    return as_block(list_get(value, 0));
}

Value* func_bindings(Value* value)
{
    return list_get(value, 1);
}

int find_first_closure_upvalue(Block* block)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->function == FUNCS.input)
            continue;
        else if (term->function == FUNCS.upvalue)
            return i;
        else
            return 0;
    }
    return 0;
}

int count_closure_upvalues(Block* block)
{
    int count = 0;
    for (int i=find_first_closure_upvalue(block); i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->function == FUNCS.upvalue)
            count++;
        else
            break;
    }
    return count;
}

void closures_install_functions(NativePatch* patch)
{
}

} // namespace circa
