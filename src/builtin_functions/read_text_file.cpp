// Copyright 2008 Andrew Fischer

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
        Term* main_func = import_function(kernel, evaluate, "read_text_file(string) : string");
        as_function(main_func).pureFunction = false;
    }
}
} // namespace circa
