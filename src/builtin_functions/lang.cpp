// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace lang_function {

    CA_FUNCTION(field)
    {
        set_ref(OUTPUT, as_branch(INPUT(0))[INPUT(1)->asInt()]);
    }

    CA_FUNCTION(num_fields)
    {
        set_int(OUTPUT, as_branch(INPUT(0)).length());
    }

    void setup(Branch& kernel)
    {
        Branch& lang_ns = create_namespace(kernel, "lang");
        import_function(lang_ns, field, "field(Branch,int) -> Ref");
        import_function(lang_ns, num_fields, "num_fields(Branch) -> int");
    }
}
}
