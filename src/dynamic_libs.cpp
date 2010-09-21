// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <dlfcn.h>

#include <branch.h>

namespace circa {

typedef void (*RegisterFunctions)(Branch* branch);

void load_dynamic_lib(Branch& branch, const char* filename)
{
    std::string file = filename;

    // TODO: platform specific
    file += ".so";

    void* dl = dlopen(file.c_str(), RTLD_LAZY);
    if (dl == NULL) {
        std::cout << "failed to load library: " << file << std::endl;
        return;
    }

    RegisterFunctions registerFunctions = (RegisterFunctions) dlsym(dl, "register_functions");

    if (registerFunctions == NULL) {
        std::cout << "could not find symbol: register_functions" << std::endl;
        dlclose(dl);
        return;
    }

    registerFunctions(&branch);

    dlclose(dl);
}

}
