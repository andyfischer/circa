// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace type_check_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(is_list, "is_list(any) -> bool")
    {
        set_bool(OUTPUT, list_t::is_list(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_int, "is_int(any) -> bool")
    {
        set_bool(OUTPUT, is_int(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_float, "is_float(any) -> bool")
    {
        set_bool(OUTPUT, is_float(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_bool, "is_bool(any) -> bool")
    {
        set_bool(OUTPUT, is_bool(INPUT(0)));
    }
    CA_DEFINE_FUNCTION(is_string, "is_string(any) -> bool")
    {
        set_bool(OUTPUT, is_string(INPUT(0)));
    }
    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }

} // namespace type_check_function
} // namespace circa
