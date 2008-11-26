// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtin_functions.h"

namespace circa {

std::vector<RegistrationFunction> gRegistrationFunctions;

RegisterBuiltinFunctionOnStartupHack::RegisterBuiltinFunctionOnStartupHack(RegistrationFunction func)
{
    gRegistrationFunctions.push_back(func);
}

void registerBuiltinFunctions(Branch& kernel)
{
    std::vector<RegistrationFunction>::iterator it;
    for (it = gRegistrationFunctions.begin(); it != gRegistrationFunctions.end(); it++) {
        (*it)(kernel);
    }
}

}
