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
    std::set<const char*> skipSet;

    TokenStream()
      : currentIndex(0)
    {
    }

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

    tokenizer::TokenInstance const& next(int lookahead=0) const;

    bool nextIs(int match, int lookahead=0) const
    {
        if ((this->currentIndex + lookahead) >= tokens.size())
            return false;
            
        return next(lookahead).match == match;
    }

    std::string consume(int match = -1);

    bool nextNonWhitespaceIs(int match, int lookahead=0) const
    {
        int index = this->currentIndex;

        while (true) {

            if (index >= (int) tokens.size())
                return false;

            if (tokens[index].match == tokenizer::WHITESPACE) {
                index++;
                continue;
            }

            if (lookahead == 0)
                return tokens[index].match == match;

            lookahead--;
            index++;
        }
    }

    bool finished() const
    {
        return (currentIndex >= tokens.size());
    }

    std::string toString() const;
};

} // namespace circa

#endif
