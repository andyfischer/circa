// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "token_stream.h"
#include "parser.h"

namespace circa {
namespace token_stream {

tokenizer::TokenInstance const&
TokenStream::next(int lookahead) const
{
    unsigned int i = this->currentIndex + lookahead;

    if (i >= tokens.size())
        parser::syntax_error("Unexpected EOF");

    return tokens[i];
}

std::string
TokenStream::consume(int match)
{
    if (finished())
        parser::syntax_error(std::string("Unexpected EOF while looking for ") + tokenizer::getMatchText(match));

    if ((match != -1) && next().match != match)
        parser::syntax_error("Unexpected token", &next());

    return tokens[currentIndex++].text;
}

} // namespace token_stream
} // namespace circa

