// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include <importing_macros.h>

using namespace circa;

extern "C" {

void on_load(Branch* branch)
{
    std::cout << "Called on_load in sampledll" << std::endl;
}

CA_FUNCTION(sample_a)
{
    set_string(OUTPUT, "patched function a()");
}

CA_FUNCTION(ns__b)
{
    set_string(OUTPUT, "patched function b()");
}

} // extern "C"
