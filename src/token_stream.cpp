// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "token_stream.h"

namespace circa {

tokenizer::TokenInstance const&
TokenStream::next(int lookahead) const
{
    unsigned int i = this->currentIndex + lookahead;

    if (i >= tokens.size())
        throw std::runtime_error("unexpected EOF");

    return tokens[i];
}

int
TokenStream::nextNonWhitespace(int lookahead) const
{
    int index = this->currentIndex;

    while (true) {

        if (index >= (int) tokens.size())
            return tokenizer::EOF_TOKEN;

        if (tokens[index].match == tokenizer::WHITESPACE) {
            index++;
            continue;
        }

        if (lookahead == 0)
            return tokens[index].match;

        lookahead--;
        index++;
    }
}

std::string
TokenStream::consume(int match)
{
    if (finished())
        std::runtime_error(std::string("Unexpected EOF while looking for ") + tokenizer::getMatchText(match));

    if ((match != -1) && next().match != match) {
        std::stringstream msg;
        msg << "Unexpected token (expected " << tokenizer::getMatchText(match)
            << ", found " << tokenizer::getMatchText(next().match)
            << " '" << next().text << "')";
        std::runtime_error(msg.str());
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

