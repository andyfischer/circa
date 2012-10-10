// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <circa/circa.h>

extern "C" {

void sample_a(caStack* stack)
{
    circa_set_string(circa_output(stack, 0), "patched sample_a");
}

void sample_b(caStack* stack)
{
    circa_set_string(circa_output(stack, 0), "patched sample_b");
}

void circa_module_load(caNativeModule* module)
{
    circa_module_patch_function(module, "sample_a", sample_a);
    circa_module_patch_function(module, "ns::sample_b", sample_b);
}

} // extern "C"
