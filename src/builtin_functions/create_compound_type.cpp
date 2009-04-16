// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace create_compound_type_function {

    void evaluate(Term* caller)
    {
        std::string name = as_string(caller->input(0));
        Type& output = as_type(caller);
        initialize_compound_type(output);
        output.name = name;
    }

    void setup(Branch& kernel)
    {
        Term* main = import_function(kernel, evaluate, "create_compound_type(string) -> Type");
        as_function(main).pureFunction = true;
    }
}
} // namespace circa
