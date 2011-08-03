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
    set_string(OUTPUT, "hi");
}

CA_FUNCTION(ns__concat)
{
    set_string(OUTPUT, std::string(STRING_INPUT(0)) + STRING_INPUT(1));
}

CA_FUNCTION(get_number)
{
    set_int(OUTPUT, 0);
}

} // extern "C"
