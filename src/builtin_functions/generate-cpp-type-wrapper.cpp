// Copyright 2008 Paul Hodge

#include "branch.h"
#include "circa.h"
#include "introspection.h"

namespace circa {
namespace generate_cpp_type_wrapper_function {

    void evaluate(Term* caller)
    {
        Type& type = as_type(caller->input(0));
        std::stringstream out;

        out << "// Generated file\n";

        out << "class " << type.name << " {\n";

    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function generate-cpp-type-wrapper(Type) -> string");
        as_function(main_func).pureFunction = true;
    }
}
}
