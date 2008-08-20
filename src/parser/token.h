#ifndef CIRCA__TOKENIZER__INCLUDED
#define CIRCA__TOKENIZER__INCLUDED

#include "common_headers.h"

namespace circa {

namespace token {

struct TokenInstance
{
    const char * match;
    std::string text;
};

typedef std::vector<TokenInstance> TokenList;

extern const char * LPAREN;
extern const char * RPAREN;
extern const char * COMMA;
extern const char * EQUALS;
extern const char * IDENTIFIER;
extern const char * INTEGER;
extern const char * FLOAT;
extern const char * STRING;
extern const char * QUOTED_IDENTIFIER;
extern const char * WHITESPACE;
extern const char * NEWLINE;
extern const char * UNRECOGNIZED;

void tokenize(std::string const &input, TokenList &results);

} // namespace token

} // namespace circa

#endif
