#ifndef CIRCA__TOKEN_STREAM__INCLUDED
#define CIRCA__TOKEN_STREAM__INCLUDED

#include "errors.h"

namespace token {

struct TokenStream
{
    TokenList const& tokens;
    int currentIndex;

    TokenStream(TokenList const& _tokens)
      : tokens(_tokens), currentIndex(0)
    {
    }

    TokenInstance const& next() const
    {
        if (finished())
            throw errors::CircaError("Ran out of tokens");

        return tokens[currentIndex];
    }

    bool nextIs(const char * match) const
    {
        return next().match == match;
    }

    std::string consume(const char * match)
    {
        if (finished())
            throw errors::CircaError("Ran out of tokens");

        if (next().match != match)
            throw errors::TokenMismatch();

        return tokens[currentIndex++].text;
    }

    bool finished() const
    {
        return (currentIndex >= tokens.size());
    }
};

}

#endif
