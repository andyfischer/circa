// Copyright 2008 Andrew Fischer

#ifndef CIRCA_TOKENIZER_INCLUDED
#define CIRCA_TOKENIZER_INCLUDED

#include "common_headers.h"

namespace circa {
namespace tokenizer {

struct Token
{
    int match;
    std::string text;
    int lineStart;
    int lineEnd;
    int colStart;
    int colEnd;

    Token() : match(0), lineStart(0), lineEnd(0),
       colStart(0), colEnd(0) {}

    std::string toString() const;
};

typedef std::vector<Token> TokenList;

const int IDENTIFIER = 1;
const int INTEGER = 2;
const int HEX_INTEGER = 3;
const int FLOAT_TOKEN = 4;
const int STRING = 5;
const int COLOR = 6;

const int LPAREN = 10;
const int RPAREN = 11;
const int LBRACE = 12;
const int RBRACE = 13;
const int LBRACKET = 14;
const int RBRACKET = 15;
const int COMMA = 16;
const int AMPERSAND = 17;
const int DOT = 18;
const int STAR = 19;
const int QUESTION = 20;
const int SLASH = 21;
const int DOUBLE_SLASH = 44;
const int PLUS = 22;
const int MINUS = 23;
const int LTHAN = 24;
const int LTHANEQ = 25;
const int GTHAN = 26;
const int GTHANEQ = 27;
const int PERCENT = 41;
const int COLON = 28;
const int DOUBLE_EQUALS = 29;
const int NOT_EQUALS = 30;
const int EQUALS = 31;
const int PLUS_EQUALS = 32;
const int MINUS_EQUALS = 33;
const int STAR_EQUALS = 34;
const int SLASH_EQUALS = 35;
const int COLON_EQUALS = 36;
const int RIGHT_ARROW = 37;
const int LEFT_ARROW = 43;
const int DOUBLE_AMPERSAND = 38;
const int DOUBLE_VERTICAL_BAR = 39;
const int SEMICOLON = 40;
const int ELLIPSIS = 42;

const int DEF = 51;
const int TYPE = 52;
const int BEGIN = 64;
const int END = 53;
const int IF = 54;
const int ELSE = 55;
const int ELIF = 63;
const int FOR = 56;
const int STATE = 57;
const int RETURN = 58;
const int IN_TOKEN = 59;
const int TRUE_TOKEN = 60;
const int FALSE_TOKEN = 61;
const int DO_ONCE = 62;

const int WHITESPACE = 70;
const int NEWLINE = 71;
const int COMMENT = 72;
const int EOF_TOKEN = 73;

const int UNRECOGNIZED = 90;

const char* get_token_text(int match);
void tokenize(std::string const &input, TokenList &results);

} // namespace tokenizer
} // namespace circa

#endif
