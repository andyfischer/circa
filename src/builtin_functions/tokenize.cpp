// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace tokenize_function {

    void evaluate(Term* caller)
    {
        std::string& input = as_string(caller->input(0));
        
        as<TokenStream>(caller).reset(input);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_function(kernel, evaluate,
                "function tokenize(string) -> TokenStream");
        as_function(main_func).pureFunction = true;
    }
}
} // namespace circa
