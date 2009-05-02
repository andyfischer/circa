// Copyright 2008 Paul Hodge

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

    bool nextIs(int match, int lookahead=0) const
    {
        if ((this->currentIndex + lookahead) >= tokens.size())
            return false;
            
        return next(lookahead).match == match;
    }

    std::string consume(int match = -1);
    tokenizer::Token const& consumet();

    bool nextNonWhitespaceIs(int match, int lookahead=0) const
    {
        return nextNonWhitespace(lookahead) == match;
    }

    bool finished() const
    {
        return (currentIndex >= tokens.size());
    }

    std::string toString() const;
};

} // namespace circa

#endif
