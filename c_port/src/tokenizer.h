#ifndef CIRCA__TOKENIZER__INCLUDED
#define CIRCA__TOKENIZER__INCLUDED

#include "common_headers.h"

namespace tokens {

struct TokenInstance
{
    const char * match;
    std::string text;
};

extern const char * LPAREN;
extern const char * RPAREN;
extern const char * IDENTIFIER;
extern const char * INTEGER;
extern const char * FLOAT;
extern const char * WHITESPACE;
extern const char * NEWLINE;
extern const char * UNRECOGNIZED;

void tokenize(std::string const &input, std::vector<TokenInstance> &results);

} // namespace tokens

#endif
