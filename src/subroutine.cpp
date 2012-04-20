// Copyright (c) Andrew Fischer. See LICENSE file for license terms.
 
#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "building.h"
#include "kernel.h"
#include "evaluation.h"
#include "function.h"
#include "importing_macros.h"
#include "locals.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "names.h"
#include "update_cascades.h"
#include "token.h"
#include "term.h"
#include "type.h"

#include "subroutine.h"

namespace circa {

namespace subroutine_f {

    void format_source(StyledSource* source, Term* term)
    {
        append_phrase(source, "def ", term, TK_DEF);

        Function* func = as_function(term);

        function_format_header_source(source, func);

        if (!is_native_function(func))
            format_branch_source(source, nested_contents(term), term);
    }
}

CA_FUNCTION(evaluate_subroutine)
{
    // This once did something, but now the default function calling behavior
    // is the same as evaluating a subroutine.
}

bool is_subroutine(Term* term)
{
    if (!is_function(term))
        return false;
    return as_function(term)->evaluate == evaluate_subroutine;
}

Term* find_enclosing_subroutine(Term* term)
{
    while (true) {
        Term* parent = get_parent_term(term);
        if (parent == NULL)
            return NULL;
        if (is_subroutine(parent))
            return parent;

        term = parent;
    }
}

int get_input_index_of_placeholder(Term* inputPlaceholder)
{
    ca_assert(inputPlaceholder->function == FUNCS.input);
    return inputPlaceholder->index - 1;
}

Term* create_function(Branch* branch, const char* name)
{
    Term* term = create_value(branch, &FUNCTION_T, name);
    initialize_function(term);
    initialize_subroutine(term);
    return term;
}

void initialize_subroutine(Term* sub)
{
    // Install evaluate function
    as_function(sub)->evaluate = evaluate_subroutine;
}

void finish_building_subroutine(Term* sub, Term* outputType)
{
    finish_update_cascade(nested_contents(sub));
}

} // namespace circa
