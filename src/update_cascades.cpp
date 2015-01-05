// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "inspection.h"
#include "interpreter.h"
#include "function.h"
#include "kernel.h"
#include "tagged_value.h"
#include "term.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

void on_block_inputs_changed(Block* block)
{
    // This once did something.
}

void fix_forward_function_references(Block* block)
{
    for (BlockIterator it(block); it; ++it) {
        Term* term = *it;
        if (term->function == NULL || term->function == FUNCS.unknown_function) {
            // See if we can now find this function
            Value* functionName = term->getProp(s_Syntax_FunctionName);

            if (functionName != NULL) {
                Term* func = find_name(block, functionName, s_LookupFunction);
                if (func != NULL)
                    change_function(term, func);
            }
        }
    }
}

} // namespace circa
