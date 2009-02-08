// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "token_stream.h"
#include "compilation.h"

namespace circa {

tokenizer::TokenInstance const&
TokenStream::next(int lookahead) const
{
    unsigned int i = this->currentIndex + lookahead;

    if (i >= tokens.size())
        syntax_error("Unexpected EOF");

    return tokens[i];
}

std::string
TokenStream::consume(int match)
{
    if (finished())
        syntax_error(std::string("Unexpected EOF while looking for ") + tokenizer::getMatchText(match));

    if ((match != -1) && next().match != match) {
        std::stringstream msg;
        msg << "Unexpected token (expected " << tokenizer::getMatchText(match)
            << ", found " << tokenizer::getMatchText(next().match)
            << " '" << next().text << "')";
        syntax_error(msg.str(), &next());
    }

    return tokens[currentIndex++].text;
}

std::string
TokenStream::toString() const
{
    std::stringstream out;

    out << "{index: " << currentIndex << ", ";
    out << "tokens: [";

    bool first = true;

    for (unsigned int i=0; i < tokens.size(); i++) {
        if (!first) out << ", ";
        out << tokens[i].toString();
        first = false;
    }
    out << "]}";
    return out.str();
}

} // namespace circa

