
#include "common_headers.h"

#include "builtins.h"
#include "codeunit.h"
#include "errors.h"
#include "globals.h"

Term* GetGlobal(string name)
{
    if (KERNEL->containsName(name))
        return KERNEL->getNamed(name);

    throw errors::KeyError(name);
}

void empty_execute_function(Term* caller)
{
}
