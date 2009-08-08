// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace create_compound_type_function {

    void evaluate(Term* caller)
    {
        std::string name = as_string(caller->input(0));
        initialize_compound_type(caller);
        as_type(caller).name = name;
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "create_compound_type(string) : Type");
    }
}
} // namespace circa
