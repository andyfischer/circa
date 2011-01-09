// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>
#include <importing_macros.h>

namespace circa {
namespace instance_function {
    
    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(instance, "instance(Type t) -> any")
    {
        change_type(OUTPUT, unbox_type(INPUT(0)));
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
        set_specialize_type(kernel["instance"], specializeType);
    }
}
}
