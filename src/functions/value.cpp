// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

namespace circa {
namespace value_function {

    CA_FUNCTION(evaluate)
    {
        if (OUTPUT != NULL)
            copy(CALLER, OUTPUT);
    }

    void setup(Branch& kernel)
    {
    }
}
}
