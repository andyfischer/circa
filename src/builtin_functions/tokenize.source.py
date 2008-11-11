# Copyright 2008 Paul Hodge

header = "function tokenize(string) -> TokenStream"
pure = True
evaluate = """
        std::string& input = as_string(caller->inputs[0]);

        as<token_stream::TokenStream>(caller).reset(input);
"""
