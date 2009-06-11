// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {
namespace write_text_file_function {

    void evaluate(Term* caller)
    {
        std::string filename = as_string(caller->input(0));
        std::string contents = as_string(caller->input(1));
        std::ofstream file;
        file.open(filename.c_str(), std::ios::out);
        file << contents;
        file.close();
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "write_text_file(string, string)");
    }
}
} // namespace circa
