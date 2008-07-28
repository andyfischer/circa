#ifndef CIRCA__TOKEN_STREAM__INCLUDED
#define CIRCA__TOKEN_STREAM__INCLUDED

#include "errors.h"

namespace token {

struct TokenStream
{
    TokenList tokens;
    int currentIndex;

    TokenStream(TokenList const& _tokens)
      : tokens(_tokens), currentIndex(0)
    {
    }

    void stripWhitespace()
    {
        int deleteCount = 0;

        for (int i=0; i < tokens.size(); i++) {

            if (tokens[i].match == WHITESPACE) {
                deleteCount++;
            } else if (deleteCount > 0) {
                tokens[i - deleteCount] = tokens[i];
            }
        }

        tokens.resize(tokens.size() - deleteCount);
    }

    TokenInstance const& next(int lookahead=0) const
    {
        int i = currentIndex + lookahead;

        if (i >= tokens.size())
            throw errors::CircaError("Ran out of tokens");

        return tokens[i];
    }

    bool nextIs(const char * match, int lookahead=0) const
    {
        return next(lookahead).match == match;
    }

    std::string consume(const char * match)
    {
        if (finished())
            throw errors::CircaError("Ran out of tokens");

        if (next().match != match)
            throw errors::TokenMismatch(match, next().match);

        return tokens[currentIndex++].text;
    }

    bool finished() const
    {
        return (currentIndex >= tokens.size());
    }
};

}

#endif
