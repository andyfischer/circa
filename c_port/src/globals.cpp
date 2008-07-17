
#include "common_headers.h"

#include "bootstrap.h"
#include "codeunit.h"
#include "globals.h"

Term* GetGlobal(string name)
{
    if (KERNEL->containsName(name))
        return KERNEL->getNamed(name);

    return NULL;
}

Term* GetGlobal(const char* name)
{
    return GetGlobal(string(name));
}
