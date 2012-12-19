// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <circa/circa.h>

void sample_a(caStack* stack)
{
    circa_set_string(circa_output(stack, 0), "patched sample_a");
}

void sample_b(caStack* stack)
{
    circa_set_string(circa_output(stack, 0), "patched sample_b");
}

CIRCA_EXPORT const char* circa_api_version()
{
    return "1";
}

CIRCA_EXPORT void circa_module_load(caNativePatch* module)
{
    circa_patch_function(module, "sample_a", sample_a);
    circa_patch_function(module, "ns:sample_b", sample_b);
}
