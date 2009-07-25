// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace ref_function {

    void get_ref(Term* caller)
    {
        as_ref(caller) = caller->input(0);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, get_ref, "get_ref(any) : ref");
    }
}
} // namespace circa
