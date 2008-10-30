// Copyright 2008 Andrew Fischer

#include "tokenizer.h"
#include "token_stream.h"

namespace tokenize_function {

    void evaluate(Term* caller)
    {
        std::string& input = as_string(caller->inputs[0]);

        as<token_stream::TokenStream>(caller).reset(input);
    }

    void setup(Branch& kernel)
    {
        Term* tokenStreamType = 
            quick_create_cpp_type<token_stream::TokenStream>(&kernel, "TokenStream");
        register_cpp_toString<token_stream::TokenStream>(tokenStreamType);

        import_c_function(kernel, evaluate,
            "function tokenize(string) -> TokenStream");
    }
}
