// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace write_text_file_function {

    CA_FUNCTION(evaluate)
    {
        std::string filename = STRING_INPUT(0);
        std::string contents = STRING_INPUT(1);
        std::ofstream file;
        file.open(filename.c_str(), std::ios::out);
        file << contents;
        file.close();
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate,
            "write_text_file(string filename, string contents);"
            "'Write contents to the given filename, overwriting any existing file' end");
    }
}
} // namespace circa
