// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_TOKEN_STREAM_INCLUDED
#define CIRCA_TOKEN_STREAM_INCLUDED

#include <set>

#include "token.h"

namespace circa {

struct TokenStream
{
    token::TokenList tokens;
    unsigned int _position;

    TokenStream()
      : _position(0)
    {

    }

    TokenStream(token::TokenList const& _tokens)
      : tokens(_tokens), _position(0)
    {
    }

    TokenStream(std::string const& input)
      : _position(0)
    {
        token::tokenize(input, tokens);
    }

    token::Token operator[](int index) {
        return tokens[index];
    }

    void reset(std::string const& input)
    {
        tokens.clear();
        token::tokenize(input, tokens);
        _position = 0;
    }

    int length() const { return (int) tokens.size(); }
    int remaining() const { return (int) tokens.size() - _position; }

    token::Token const& next(int lookahead=0) const;

    int nextNonWhitespace(int lookahead=0) const;

    bool nextIs(int match, int lookahead=0) const;

    // Consume the next token and return its text contents. If a token match is provided,
    // and the next token doesn't have this match, then we throw an exception. This check
    // should only be used as a form of assert; an expected match failure should be
    // caught ahead of time.
    std::string consume(int match = -1);

    bool nextNonWhitespaceIs(int match, int lookahead=0) const;

    bool finished() const
    {
        return (_position >= tokens.size());
    }

    int getPosition() const;
    void resetPosition(int loc); 
    std::string toString() const;
};

void print_remaining_tokens(std::ostream& stream, TokenStream& tokens);

} // namespace circa

#endif
