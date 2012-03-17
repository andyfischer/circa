// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "../file_utils.h"

namespace circa {
namespace write_text_file_function {

    CA_FUNCTION(evaluate)
    {
        circa_write_text_file(STRING_INPUT(0), STRING_INPUT(1));
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate,
            "write_text_file(string filename, string contents);"
            "'Write contents to the given filename, overwriting any existing file'");
    }
}
} // namespace circa
