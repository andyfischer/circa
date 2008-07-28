
#include "common_headers.h"

#include "parser/token.h"
#include "parser/token_stream.h"
#include "globals.h"
#include "operations.h"
#include "bootstrapping.h"


Term* quick_eval_function(Branch* branch, std::string input)
{
    token::TokenList tokenList;
    token::tokenize(input, tokenList);
    token::TokenStream tstream(tokenList);

    Term* function = find_named(branch, tstream.consume(token::IDENTIFIER));

    tstream.consume(token::LPAREN);

    TermList inputs;

    while(!tstream.nextIs(token::RPAREN)) {

        if (tstream.nextIs(token::IDENTIFIER)) {
            Term* term = find_named(branch, tstream.consume(token::IDENTIFIER));
            inputs.append(term);

        } else if (tstream.nextIs(token::STRING)) {

        }
    }

    tstream.consume(token::RPAREN);

    return apply_function(branch, function, inputs);
}

