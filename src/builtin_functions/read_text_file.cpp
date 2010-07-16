// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace read_text_file_function {

    CA_FUNCTION(evaluate)
    {
        std::string filename = as_string(INPUT(0));
        set_str(OUTPUT, storage::read_text_file(filename));
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "read_text_file(string) -> string");
    }
}
} // namespace circa
