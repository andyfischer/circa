// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "static_checking.h"
#include "term.h"

#include "types/list.h"

namespace circa {


void append_static_error(StaticErrorCheck* result, Term* term, const char* type)
{
    List* item = set_list(result->errors.append(), 2);
    set_int(item->get(0), term->index);
    set_string(item->get(1), type);
}

void check_term_for_static_error(StaticErrorCheck* result, Term* term)
{
    if (term->function == NULL)
        return append_static_error(result, term, "null_function");

    bool varArgs = function_t::get_variable_args(term->function);
    int funcNumInputs = function_t::num_inputs(term->function);

    // Check # of inputs
    if (!varArgs && (term->inputs.length() != funcNumInputs))
        return append_static_error(result, term, "wrong_input_count");

    // TODO: Errors related to inputs

    // This next section includes expected parser errors

    // Unknown function
    if (term->function == UNKNOWN_FUNCTION)
        return append_static_error(result, term, "unknown_function");

    // Unknown type
    if (term->function == UNKNOWN_TYPE_FUNC)
        return append_static_error(result, term, "unknown_type");

    // Unknown identifier
    if (term->function == UNKNOWN_IDENTIFIER_FUNC)
        return append_static_error(result, term, "unknown_identifier");

    // Unrecognized expression
    if (term->function == UNRECOGNIZED_EXPRESSION_FUNC)
        return append_static_error(result, term, "unrecognized_expression");
}

void check_for_static_errors(StaticErrorCheck* result, Branch& branch)
{
    for (int i=0; i < branch.length(); i++)
        check_term_for_static_error(result, branch[i]);
}

} // namespace circa
