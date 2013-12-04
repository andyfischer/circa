// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "block.h"
#include "building.h"
#include "bytecode.h"
#include "code_iterators.h"
#include "inspection.h"
#include "interpreter.h"
#include "function.h"
#include "heap_debugging.h"
#include "kernel.h"
#include "tagged_value.h"
#include "term.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

void mark_static_errors_invalid(Block* block)
{
    set_null(&block->staticErrors);
}

void on_block_inputs_changed(Block* block)
{
}

void fix_forward_function_references(Block* block)
{
    for (BlockIterator it(block); it.unfinished(); it.advance()) {
        Term* term = *it;
        if (term->function == NULL || term->function == FUNCS.unknown_function) {
            // See if we can now find this function
            std::string functionName = term->stringProp(sym_Syntax_FunctionName, "");

            Term* func = find_name(block, functionName.c_str(), sym_LookupFunction);
            if (func != NULL)
                change_function(term, func);
        }
    }
}

void dirty_bytecode(Block* block)
{
    set_null(&block->bytecode);

    // Dirty nested minor blocks.
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->nestedContents != NULL && is_minor_block(term->nestedContents))
            dirty_bytecode(term->nestedContents);
    }
}

void refresh_bytecode(Block* block)
{
    // Don't bother touching bytecode if we're still bootstrapping.
    if (global_world()->bootstrapStatus == sym_Bootstrapping)
        return;

    if (is_null(&block->bytecode))
        bytecode_write_block(&block->bytecode, block);
}

} // namespace circa
