// Copyright 2008 Paul Hodge

#ifndef CIRCA_TOKENIZER_INCLUDED
#define CIRCA_TOKENIZER_INCLUDED

#include "common_headers.h"

namespace circa {
namespace tokenizer {

struct TokenInstance
{
    int match;
    std::string text;
    int line;
    int character;

    TokenInstance() : match(0), line(0), character(0) {}
};

typedef std::vector<TokenInstance> TokenList;

const int LPAREN = 1;
const int RPAREN = 2;
const int LBRACE = 30;
const int RBRACE = 31;
const int LBRACKET = 34;
const int RBRACKET = 35;
const int COMMA = 3;
const int AMPERSAND = 32;

const int IDENTIFIER = 4;
const int INTEGER = 5;
const int FLOAT = 6;
const int STRING = 7;
const int QUOTED_IDENTIFIER = 8;

const int DOT = 9;
const int STAR = 10;
const int QUESTION = 33;
const int SLASH = 11;
const int PLUS = 12;
const int MINUS = 13;
const int LTHAN = 14;
const int LTHANEQ = 15;
const int GTHAN = 16;
const int GTHANEQ = 17;
const int DOUBLE_EQUALS = 18;
const int NOT_EQUALS = 19;
const int EQUALS = 20;
const int PLUS_EQUALS = 21;
const int MINUS_EQUALS = 22;
const int STAR_EQUALS = 23;
const int SLASH_EQUALS = 24;
const int RIGHT_ARROW = 25;

const int WHITESPACE = 26;
const int NEWLINE = 27;
const int UNRECOGNIZED = 28;

const char* getMatchText(int match);
void tokenize(std::string const &input, TokenList &results);

} // namespace tokenizer
} // namespace circa

#endif
