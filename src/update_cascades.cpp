// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "branch.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "heap_debugging.h"
#include "function.h"
#include "kernel.h"
#include "locals.h"
#include "building.h"
#include "tagged_value.h"
#include "term.h"
#include "update_cascades.h"

namespace circa {

void mark_static_errors_invalid(Branch* branch)
{
    set_null(&branch->staticErrors);
}

void on_create_call(Term* term)
{
    if (!is_function(term->function))
        return;

    Function::OnCreateCall func = as_function(term->function)->onCreateCall;

    if (func)
        func(term);
}

void on_term_name_changed(Term* term, const char* oldName, const char* newName)
{
    // No-op for now
}

void on_branch_inputs_changed(Branch* branch)
{
}

void fix_forward_function_references(Branch* branch)
{
    for (BranchIterator it(branch); it.unfinished(); it.advance()) {
        Term* term = *it;
        if (term->function == NULL || term->function == FUNCS.unknown_function) {
            // See if we can now find this function
            std::string functionName = term->stringProp("syntax:functionName", "");

            Term* func = find_name(branch, functionName.c_str());
            if (func != NULL) {
                change_function(term, func);
            }
        }
    }
}

void dirty_bytecode(Branch* branch)
{
    set_null(&branch->bytecode);
}

void refresh_bytecode(Branch* branch)
{
    if (is_null(&branch->bytecode))
        write_branch_bytecode(branch, &branch->bytecode);
}

} // namespace circa
