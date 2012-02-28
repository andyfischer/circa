// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include <vector>

namespace circa {

struct Token
{
    int match;
    int charIndex;
    int lineStart;
    int lineEnd;
    int colStart;
    int colEnd;
    int precedingIndent;

    Token() : match(0), charIndex(0), lineStart(0), lineEnd(0), colStart(0), colEnd(0), precedingIndent(0) {}

    std::string toString() const;
    int length() const;
};

typedef std::vector<Token> TokenList;

const int TK_IDENTIFIER = 1;
const int TK_SYMBOL = 9;
const int TK_INTEGER = 3;
const int TK_HEX_INTEGER = 4;
const int TK_FLOAT = 5;
const int TK_STRING = 6;
const int TK_COLOR = 7;
const int TK_BOOL = 8;

const int TK_LPAREN = 10;
const int TK_RPAREN = 11;
const int TK_LBRACE = 12;
const int TK_RBRACE = 13;
const int TK_LBRACKET = 14;
const int TK_RBRACKET = 15;
const int TK_COMMA = 16;
const int TK_AT_SIGN = 17;
const int TK_DOT = 18;
const int TK_STAR = 19;
const int TK_QUESTION = 20;
const int TK_SLASH = 21;
const int TK_DOUBLE_SLASH = 44;
const int TK_PLUS = 22;
const int TK_MINUS = 23;
const int TK_LTHAN = 24;
const int TK_LTHANEQ = 25;
const int TK_GTHAN = 26;
const int TK_GTHANEQ = 27;
const int TK_PERCENT = 41;
const int TK_COLON = 28;
const int TK_DOUBLE_COLON = 47;
const int TK_DOUBLE_EQUALS = 29;
const int TK_NOT_EQUALS = 30;
const int TK_EQUALS = 31;
const int TK_PLUS_EQUALS = 32;
const int TK_MINUS_EQUALS = 33;
const int TK_STAR_EQUALS = 34;
const int TK_SLASH_EQUALS = 35;
const int TK_COLON_EQUALS = 36;
const int TK_RIGHT_ARROW = 37;
const int TK_LEFT_ARROW = 43;
const int TK_AMPERSAND = 45;
const int TK_DOUBLE_AMPERSAND = 38;
const int TK_DOUBLE_VERTICAL_BAR = 39;
const int TK_SEMICOLON = 40;
const int TK_TWO_DOTS = 46;
const int TK_ELLIPSIS = 42;
const int TK_TRIPLE_LTHAN = 48;
const int TK_TRIPLE_GTHAN = 49;
const int TK_POUND = 50;

const int TK_DEF = 51;
const int TK_TYPE = 52;
const int TK_BEGIN = 64;
const int TK_DO = 70;
const int TK_END = 53;
const int TK_IF = 54;
const int TK_ELSE = 55;
const int TK_ELIF = 63;
const int TK_FOR = 56;
const int TK_STATE = 57;
const int TK_RETURN = 58;
const int TK_IN = 59;
const int TK_TRUE = 60;
const int TK_FALSE = 61;
const int TK_DO_ONCE = 62;
const int TK_NAMESPACE = 65;
const int TK_INCLUDE = 66;
const int TK_IMPORT = 76;
const int TK_AND = 67;
const int TK_OR = 68;
const int TK_DISCARD = 69;
const int TK_NULL = 71;
const int TK_BREAK = 72;
const int TK_CONTINUE = 73;
const int TK_SWITCH = 74;
const int TK_CASE = 75;
const int TK_WHILE = 77;

const int TK_WHITESPACE = 80;
const int TK_NEWLINE = 81;
const int TK_COMMENT = 82;
const int TK_EOF = 83;

const int TK_UNRECOGNIZED = 90;

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

    // Like consume(), but saves the text of the consumed token in a caValue.
    void consumeStr(caValue* output, int match = -1);

    // Like consume(), but registers the string as a runtime symbol.
    caName consumeName(int match = -1);

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
