// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "kernel.h"
#include "if_block.h"
#include "term.h"
#include "type.h"

namespace circa {

void switch_block_post_compile(Term* term)
{
    // Add a default case
    apply(nested_contents(term), FUNCS.default_case, TermList());
    finish_if_block(term);
}

void evaluate_switch(caStack* stack)
{
}

} // namespace circa
