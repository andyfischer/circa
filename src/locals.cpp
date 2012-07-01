// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "kernel.h"
#include "function.h"
#include "inspection.h"
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
    return function_num_outputs(attrs);
}

int get_locals_count(Branch* branch)
{
    return branch->length();
}

} // namespace circa
