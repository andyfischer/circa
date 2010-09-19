// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace inspect_function {

    CA_FUNCTION(get_raw)
    {
        make_string(OUTPUT, get_branch_raw(INPUT_TERM(0)->nestedContents));
    }

    void setup(Branch& kernel)
    {
        Branch& inspect_ns = create_namespace(kernel, "inspect");
        import_function(inspect_ns, get_raw, "get_raw(Branch)->string");
    }
}
}
