// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace map_function {

    CA_FUNCTION(evaluate)
    {
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "def map(any,Indexable) -> List");
    }
}
} // namespace circa
