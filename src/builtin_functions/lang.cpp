// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <circa.h>

namespace circa {
namespace lang_function {

    void field(Term* caller)
    {
        as_ref(caller) = as_branch(caller->input(0))[caller->input(1)->asInt()];
    }

    void num_fields(Term* caller)
    {
        as_int(caller) = as_branch(caller->input(0)).length();
    }

    void setup(Branch& kernel)
    {
        Branch& lang_ns = create_namespace(kernel, "lang");
        import_function(lang_ns, field, "field(Branch,int) : Ref");
        import_function(lang_ns, num_fields, "num_fields(Branch) : int");
    }
}
}
