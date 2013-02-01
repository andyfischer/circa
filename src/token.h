// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include <vector>

namespace circa {

struct Token
{
    int match;
    int start;
    int end;
    int lineStart;
    int lineEnd;
    int colStart;
    int colEnd;
    int precedingIndent;

    Token() : match(0), start(0), end(0), lineStart(0), lineEnd(0),
        colStart(0), colEnd(0), precedingIndent(0) {}

    std::string toString() const;
    int length() const;
};

typedef std::vector<Token> TokenList;

const char* get_token_text(int match);
void tokenize(std::string const &input, TokenList* results);

struct TokenStream
{
    std::string _sourceText;
    TokenList tokens;
    unsigned int _position;

    TokenStream()
      : _position(0)
    {
    }

    TokenStream(TokenList const& _tokens)
      : tokens(_tokens), _position(0)
    {
    }

    TokenStream(std::string const& input)
      : _sourceText(input), _position(0)
    {
        tokenize(input, &tokens);
    }

    Token operator[](int index) {
        return tokens[index];
    }

    void reset(caValue* inputString);

    void reset(std::string const& input)
    {
        _sourceText = input;
        tokens.clear();
        tokenize(input, &tokens);
        _position = 0;
    }

    int length() const { return (int) tokens.size(); }
    int remaining() const { return (int) tokens.size() - _position; }
    int position() const { return _position; }

    Token const& next(int lookahead=0) const;
    std::string nextStr(int lookahead=0) const;
    void getNextStr(caValue* value, int lookahead) const;

    // Return true if the given lookahead is past the end of the list.
    bool nextIsEof(int lookahead) const;

    bool nextIs(int match, int lookahead=0) const;
    bool nextEqualsString(const char* str, int lookahead=0) const;

    Symbol nextMatch(int lookahead=0) const;

    // Consume the next token. If a token match is provided, and the next token doesn't
    // have this match, then we trigger a fatal error. This matching should be treated
    // like an assert().
    void consume(int match = -1);

    // Like consume(), but also returns the text of the consumed token.
    std::string consumeStr(int match = -1);

    // Like consume(), but appends the text of the consumed token to a caValue.
    void consumeStr(caValue* output, int match = -1);

    bool finished() const
    {
        return (_position >= tokens.size());
    }

    int getPosition() const;
    void resetPosition(int loc); 
    std::string toString() const;
    void dump();
};

void print_remaining_tokens(std::ostream& stream, TokenStream& tokens);

} // namespace circa
