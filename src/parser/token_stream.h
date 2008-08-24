#ifndef CIRCA__TOKEN_STREAM__INCLUDED
#define CIRCA__TOKEN_STREAM__INCLUDED

#include "syntax_errors.h"
#include "token.h"

namespace circa {
namespace parser {

struct TokenStream
{
    token::TokenList tokens;
    int currentIndex;

    TokenStream(token::TokenList const& _tokens)
      : tokens(_tokens), currentIndex(0)
    {
    }

    void stripWhitespace()
    {
        int deleteCount = 0;

        for (int i=0; i < tokens.size(); i++) {

            if (tokens[i].match == token::WHITESPACE) {
                deleteCount++;
            } else if (deleteCount > 0) {
                tokens[i - deleteCount] = tokens[i];
            }
        }

        tokens.resize(tokens.size() - deleteCount);
    }

    token::TokenInstance const& next(int lookahead=0) const
    {
        int i = currentIndex + lookahead;

        if (i >= tokens.size())
            throw errors::UnexpectedEOF();

        return tokens[i];
    }

    bool nextIs(const char * match, int lookahead=0) const
    {
        return next(lookahead).match == match;
    }

    std::string consume(const char * match = NULL)
    {
        if (finished())
            throw errors::UnexpectedEOF();

        if ((match != NULL) && next().match != match)
            throw errors::UnexpectedToken(match, next().match, next().text.c_str());

        return tokens[currentIndex++].text;
    }

    bool finished() const
    {
        return (currentIndex >= tokens.size());
    }
};

} // namespace token
} // namespace circa

#endif
