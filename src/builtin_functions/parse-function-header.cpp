// Copyright 2008 Andrew Fischer

#include "ast.h"
#include "cpp_interface.h"
#include "parser.h"
#include "tokenizer.h"
#include "token_stream.h"

namespace circa {
namespace parse_function_header_function {

    void evaluate(Term* caller)
    {
        token_stream::TokenStream &tokens = as<token_stream::TokenStream>(caller->input(0));
        ast::FunctionHeader &result = as<ast::FunctionHeader>(caller);

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
    }

    void setup(Branch& kernel)
    {
        import_type<ast::FunctionHeader>(kernel, "FunctionHeader");

        Term* main_func = import_c_function(kernel, evaluate,
            "function parse-function-header(TokenStream) -> FunctionHeader");

        as_function(main_func).pureFunction = false;
    }
}
} // namespace circa
