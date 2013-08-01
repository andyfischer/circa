// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "source_repro.h"
#include "tagged_value.h"

namespace circa {

void closure_redirect_outside_references(Block* block)
{
    TermMap outerToInnerMap;

    int nextInsertPosition = count_input_placeholders(block);

    for (BlockIterator it(block); it.unfinished(); it.advance()) {
        Term* innerTerm = *it;
        for (int inputIndex=0; inputIndex < innerTerm->numInputs(); inputIndex++) {
            Term* input = innerTerm->input(inputIndex);
            if (input == NULL)
                continue;

            if (!term_is_nested_in_block(input, block)) {
                // This is an outer reference

                // Check if we've already created an input for this one
                Term* existingPlaceholder = outerToInnerMap[input];

                if (existingPlaceholder != NULL) {
                    remap_pointers_quick(innerTerm, input, existingPlaceholder);

                } else {
                    // Create a new unbound_input term.
                    Term* unbound = apply(block, FUNCS.unbound_input,
                        TermList(input), &input->nameValue);
                    change_declared_type(unbound, input->type);
                    block->move(unbound, nextInsertPosition++);
                    remap_pointers_quick(innerTerm, input, unbound);
                    it.advance();
                }
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

    FUNCS.unbound_input = install_function(kernel, "unbound_input", NULL);
    block_set_evaluation_empty(function_contents(FUNCS.unbound_input), true);

    FUNCS.func_apply = kernel->get("Func.apply");
}

} // namespace circa
