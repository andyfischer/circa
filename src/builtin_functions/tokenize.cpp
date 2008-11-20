// Copyright 2008 Paul Hodge

namespace tokenize_function {

    void evaluate(Term* caller)
    {
        std::string& input = as_string(caller->inputs[0]);
        
        as<token_stream::TokenStream>(caller).reset(input);
    }

    void setup(Branch& kernel)
    {
        ast::FunctionHeader myFunctionHeader;
        myFunctionHeader.functionName = "tokenize";
        myFunctionHeader.addArgument("string", "");
        myFunctionHeader.outputType = "TokenStream";
        Term* main_func = import_c_function_manual_header(kernel, evaluate, myFunctionHeader);
        as_function(main_func).pureFunction = true;
    }
}
