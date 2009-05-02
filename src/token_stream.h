// Copyright 2008 Andrew Fischer

#ifndef CIRCA_TOKEN_STREAM_INCLUDED
#define CIRCA_TOKEN_STREAM_INCLUDED

#include <set>

#include "tokenizer.h"

namespace circa {

struct TokenStream
{
    tokenizer::TokenList tokens;
    unsigned int currentIndex;

    TokenStream(tokenizer::TokenList const& _tokens)
      : tokens(_tokens), currentIndex(0)
    {
    }

    TokenStream(std::string const& input)
      : currentIndex(0)
    {
        tokenizer::tokenize(input, tokens);
    }

    void reset(std::string const& input)
    {
        tokens.clear();
        tokenizer::tokenize(input, tokens);
        currentIndex = 0;
    }

    int length() const { return (int) tokens.size(); }

    tokenizer::Token const& next(int lookahead=0) const;

    int nextNonWhitespace(int lookahead=0) const;

    bool nextIs(int match, int lookahead=0) const;

    // Consume the next token and return its text contents. If match is not -1, then throw
    // an exception if the consumed token is not the same as match.
    // This method is deprecated for two reasons:
    //  1) Throwing exceptions is a bad way to handle errors
    //  2) We throw out token-location information by just returning a string.
    // Use consumet() instead.
    std::string consume(int match = -1);

    // Consume the next token and return it. This method is the recommended replacement
    // for consume()
    tokenizer::Token const& consumet();

    bool nextNonWhitespaceIs(int match, int lookahead=0) const;

    bool finished() const
    {
        return (currentIndex >= tokens.size());
    }

    int getLocation() const;
    void resetLocation(int loc); 
    std::string toString() const;
};

} // namespace circa

#endif
