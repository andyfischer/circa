// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace read_text_file_function {

    CA_FUNCTION(evaluate)
    {
        std::string filename = as_string(INPUT(0));
        TValue error;
        read_text_file_to_value(filename.c_str(), OUTPUT, &error);

        if (!is_null(&error))
            RAISE_ERROR(as_cstring(&error));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "read_text_file(string) -> string");
    }
}
} // namespace circa
