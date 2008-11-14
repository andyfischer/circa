// Copyright 2008 Paul Hodge

#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "token_stream.h"

namespace parse_function_header_function {

    ast::FunctionHeader main(std::string input)
    {
        token_stream::TokenStream tokens(input);

        ast::FunctionHeader result;

        std::string firstIdentifier = tokens.consume(tokenizer::IDENTIFIER); // 'function'
        parser::possibleWhitespace(tokens);

        if (firstIdentifier == "function") {
            result.functionName = tokens.consume(tokenizer::IDENTIFIER);
            parser::possibleWhitespace(tokens);
        } else {
            result.functionName = firstIdentifier;
        }

        tokens.consume(tokenizer::LPAREN);

        while (!tokens.nextIs(tokenizer::RPAREN))
        {
            std::string preWhitespace = parser::possibleWhitespace(tokens);
            std::string type = tokens.consume(tokenizer::IDENTIFIER);
            std::string innerWhitespace = parser::possibleWhitespace(tokens);

            std::string name, postWhitespace;
            if (tokens.nextIs(tokenizer::COMMA) || tokens.nextIs(tokenizer::RPAREN)) {
                name = "";
                postWhitespace = "";
            } else {
                name = tokens.consume(tokenizer::IDENTIFIER);
                postWhitespace = parser::possibleWhitespace(tokens);
            }

            result.addArgument(type, name);

            if (!tokens.nextIs(tokenizer::RPAREN))
                tokens.consume(tokenizer::COMMA);
        }

        tokens.consume(tokenizer::RPAREN);

        parser::possibleWhitespace(tokens);

        if (tokens.nextIs(tokenizer::RIGHT_ARROW)) {
            tokens.consume(tokenizer::RIGHT_ARROW);
            parser::possibleWhitespace(tokens);
            result.outputType = tokens.consume(tokenizer::IDENTIFIER);
            parser::possibleWhitespace(tokens);
        }

        return result;
    }

    void evaluate(Term* caller)
    {
        as<ast::FunctionHeader>(caller) = main(as_string(caller->inputs[0]));
    }

    void setup(Branch& kernel)
    {
        quick_create_cpp_type<ast::FunctionHeader>(kernel, "FunctionHeader");

        Term* main_func = import_c_function(kernel, evaluate,
                "function parse-function-header(string) -> FunctionHeader");
        as_function(main_func).pureFunction = false;
    }
}
