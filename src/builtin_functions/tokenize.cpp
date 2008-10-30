// Copyright 2008 Paul Hodge

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
        //quick_create_cpp_type<tokenizer::TokenList>(kernel, "TokenList");
        quick_create_cpp_type<token_stream::TokenStream>(&kernel, "TokenStream");

        import_c_function(kernel, evaluate,
            "function tokenize(string) -> TokenStream");
    }
}
