// Copyright 2008 Paul Hodge

#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "token_stream.h"

namespace circa {
namespace parse_expression_function {

    void evaluate(Term* caller)
    {
        /*
        token_stream::TokenStream &tokens = as<token_stream::TokenStream>(caller->inputs[0]);
        ast::Expression &result = as<ast::Expression>(caller);
        */
    }

    void setup(Branch& kernel)
    {
        /*
        quick_create_cpp_type<ast::Expression>(kernel, "ExpresionAST");

        Term* main_func = import_c_function(kernel, evaluate,
                "function parse-expression(TokenStream) -> ExpressionAST");
        as_function(main_func).pureFunction = false;
        */
    }
}
} // namespace circa
