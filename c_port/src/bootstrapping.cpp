
#include "common_headers.h"

#include "branch.h"
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
    tstream.stripWhitespace();

    try {

        // Check for name binding
        std::string nameBinding = "";
        if (tstream.nextIs(token::IDENTIFIER) && tstream.nextIs(token::EQUALS, 1)) {
            nameBinding = tstream.consume(token::IDENTIFIER);
            tstream.consume(token::EQUALS);
        }

        Term* function = find_named(branch, tstream.consume(token::IDENTIFIER));

        tstream.consume(token::LPAREN);

        TermList inputs;

        while(true) {

            if (tstream.nextIs(token::IDENTIFIER)) {
                Term* term = find_named(branch, tstream.consume(token::IDENTIFIER));
                inputs.append(term);
            }
            else if (tstream.nextIs(token::STRING)) {
                Term* term = constant_string(branch, tstream.consume(token::STRING));
                inputs.append(term);
            }

            if (tstream.nextIs(token::COMMA))
                tstream.consume(token::COMMA);
            else {
                tstream.consume(token::RPAREN);
                break;
            }
        }

        Term* result = apply_function(branch, function, inputs);

        if (nameBinding != "")
            branch->bindName(result, nameBinding);

        return result;

    } catch (errors::CircaError &err) {
        std::cout << "Error doing eval on: " << input << std::endl;
        std::cout << err.message() << std::endl;
        return NULL;
    }
}
