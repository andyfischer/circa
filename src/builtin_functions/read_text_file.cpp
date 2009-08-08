// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace read_text_file_function {

    void evaluate(Term* caller)
    {
        std::string filename = as_string(caller->input(0));
        as_string(caller) = read_text_file(filename);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "read_text_file(string) : string");
    }
}
} // namespace circa
