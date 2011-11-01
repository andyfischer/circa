// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "introspection.h"
#include "term.h"

#include "locals.h"

namespace circa {

int get_output_count(Term* term)
{
    if (!FINISHED_BOOTSTRAP)
        return 1;

    // check if the function has overridden getOutputCount
    Function::GetOutputCount getOutputCount = NULL;

    if (term->function == NULL)
        return 1;

    Function* attrs = as_function(term->function);

    if (attrs == NULL)
        return 1;
    
    getOutputCount = attrs->getOutputCount;

    if (getOutputCount != NULL)
        return getOutputCount(term);

    // Default behavior, if Function was found.
    return attrs->outputCount;
}

int get_locals_count(Branch* branch)
{
    return branch->length();
}

void update_output_count(Term* term)
{
    term->outputCount = get_output_count(term);
}

} // namespace circa
