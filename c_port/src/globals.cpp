
#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "globals.h"

Term* get_global(string name)
{
    if (KERNEL->containsName(name))
        return KERNEL->getNamed(name);

    throw errors::KeyError(name);
}

void empty_execute_function(Term* caller)
{
}
