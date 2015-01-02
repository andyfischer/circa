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

    int length();
};

typedef std::vector<Token> TokenList;

const char* get_token_text(int match);
bool token_is_identifier_or_keyword(int match);
void tokenize(const char* input, int len, TokenList* results);
void tokenize(std::string const &input, TokenList* results);

struct TokenStream
{
    Value _sourceText;
    TokenList tokens;
    unsigned int _position;

    TokenStream(Value* sourceText);
    TokenStream(const char* sourceText);

    Token operator[](int index) {
        return tokens[index];
    }
    Token get(int index) {
        return tokens[index];
    }

    void reset(Value* inputString);
    void reset(const char* inputString);

    int length() { return (int) tokens.size(); }
    int remaining() { return (int) tokens.size() - _position; }
    int position() { return _position; }
    bool finished() { return (_position >= tokens.size()); }

    Token& next(int lookahead=0);
    void getNextStr(Value* value, int lookahead=0);

    // Return true if the given lookahead is past the end of the list.
    bool nextIsEof(int lookahead);

    bool nextIs(int match, int lookahead=0);
    bool nextEqualsString(const char* str, int lookahead=0);

    Symbol nextMatch(int lookahead=0);
    int indentOfLine(int lookahead=0);

    // Consume the next token. If a token match is provided, and the next token doesn't
    // have this match, then we trigger a fatal error. This matching should be treated
    // like an assert().
    void consume(int match = -1);

    // Like consume(), but also returns the text of the consumed token.
    std::string consumeStr(int match = -1);

    // Like consume(), but appends the text of the consumed token to a Value.
    void consumeStr(Value* output, int match = -1);

    void dropRemainder();

    int getPosition();
    void setPosition(int loc); 
    void dump();
};

void dump_remaining_tokens(TokenStream& tokens);

} // namespace circa
