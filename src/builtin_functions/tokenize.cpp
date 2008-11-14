// Copyright 2008 Andrew Fischer

namespace tokenize_function {

    void evaluate(Term* caller)
    {
        std::string& input = as_string(caller->inputs[0]);
        
        as<token_stream::TokenStream>(caller).reset(input);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function tokenize(string) -> TokenStream");
        as_function(main_func).pureFunction = true;
    }
}
