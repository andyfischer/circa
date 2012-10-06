// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <vector>
#include <map>

#include "native_modules.h"

namespace circa {

struct NativeModuleWorld
{
    std::map<Name, NativeModule*> nativeModules;
};

struct NativePatchFunction
{
    Name name;
    EvaluateFunc evaluateFunc;
};

struct NativeModule
{
    std::vector<NativePatchFunction> nativePatchFunctions;
    Name affectsNamespace;

    // If this module was loaded from a DLL or shared object, that object is here.
    // May be NULL if the module was created a different way.
    void* dll;
};

NativeModuleWorld* create_native_module_world()
{
    return new NativeModuleWorld();
}

} // namespace circa
