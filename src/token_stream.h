// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include <set>

#include "token.h"

namespace circa {

struct TokenStream
{
    std::string _sourceText;
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
      : _sourceText(input), _position(0)
    {
        token::tokenize(input, &tokens);
    }

    token::Token operator[](int index) {
        return tokens[index];
    }

    void reset(std::string const& input)
    {
        _sourceText = input;
        tokens.clear();
        token::tokenize(input, &tokens);
        _position = 0;
    }

    int length() const { return (int) tokens.size(); }
    int remaining() const { return (int) tokens.size() - _position; }
    int position() const { return _position; }

    token::Token const& next(int lookahead=0) const;
    std::string nextStr(int lookahead=0) const;

    // Returns the position of the next non-whitespace token. Returns -1 if
    // the end of the stream was reached before anything was found.
    int findNextNonWhitespace(int lookahead=0) const;

    // Returns the token type of the next non-whitespace token.
    int nextNonWhitespace(int lookahead=0) const;

    bool nextIs(int match, int lookahead=0) const;

    // Consume the next token. If a token match is provided, and the next token doesn't
    // have this match, then we trigger a fatal error. This matching should be treated
    // like an assert().
    void consume(int match = -1);

    // Like consume(), but also returns the text of the consumed token.
    std::string consumeStr(int match = -1);

    // Like consume(), but registers the string as a runtime symbol.
    Symbol consumeSymbol(int match = -1);

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
