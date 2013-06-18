// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "file.h"

namespace circa {
namespace write_text_file_function {

    void evaluate(caStack* stack)
    {
        write_text_file(circa_string_input(stack, 0), circa_string_input(stack, 1));
    }

    void setup(Block* kernel)
    {
        import_function(kernel, evaluate,
            "write_text_file(String filename, String contents);"
            "'Write contents to the given filename, overwriting any existing file'");
    }
}
} // namespace circa
