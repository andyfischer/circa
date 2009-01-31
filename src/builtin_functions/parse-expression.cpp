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
        TokenStream &tokens = as<TokenStream>(caller->input(0));
        ast::Expression &result = as<ast::Expression>(caller);
        */
    }

    void setup(Branch& kernel)
    {
        /*
        import_type<ast::Expression>(kernel, "ExpressionAST");

        Term* main_func = import_function(kernel, evaluate,
                "function parse-expression(TokenStream) -> ExpressionAST");
        as_function(main_func).pureFunction = false;
        */
    }
}
} // namespace circa
