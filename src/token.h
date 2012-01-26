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

const int TK_WHITESPACE = 80;
const int TK_NEWLINE = 81;
const int TK_COMMENT = 82;
const int TK_EOF = 83;

const int TK_UNRECOGNIZED = 90;

const char* get_token_text(int match);
void tokenize(std::string const &input, TokenList* results);

} // namespace circa
