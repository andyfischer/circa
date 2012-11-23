// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "importing.h"
#include "kernel.h"
#include "list.h"
#include "source_repro.h"

namespace circa {

void closure_redirect_outside_references(Block* block)
{
    // TODO
}

void closure_block_formatSource(caValue* source, Term* term)
{
    format_name_binding(source, term);
    format_block_source(source, nested_contents(term), term);
}

void closure_block_evaluate(caStack* stack)
{
    // Capture bindings
    // TODO.

    Term* term = circa_caller_term(stack);
    caValue* closureOutput = circa_create_default_output(stack, 0);
    set_block(list_get(closureOutput, 0), nested_contents(term));
    set_list(list_get(closureOutput, 1), 0);
}

void closures_install_functions(Block* kernel)
{
    FUNCS.closure_block = install_function(kernel, "closure_block", closure_block_evaluate);
    as_function(FUNCS.closure_block)->formatSource = closure_block_formatSource;
}

}
