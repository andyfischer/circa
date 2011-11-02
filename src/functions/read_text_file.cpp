// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace read_text_file_function {

    CA_FUNCTION(evaluate)
    {
        std::string filename = as_string(INPUT(0));
        TaggedValue error;
        read_text_file_to_value(filename.c_str(), OUTPUT, &error);

        if (!is_null(&error))
            ERROR_OCCURRED(as_cstring(&error));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate, "read_text_file(string) -> string");
    }
}
} // namespace circa
