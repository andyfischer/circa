// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "token_stream.h"

namespace circa {

tokenizer::Token const&
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

bool TokenStream::nextIs(int match, int lookahead) const
{
    if ((this->currentIndex + lookahead) >= tokens.size())
        return false;
        
    return next(lookahead).match == match;
}

std::string
TokenStream::consume(int match)
{
    if (finished())
        throw std::runtime_error(std::string("Unexpected EOF while looking for ")
                + tokenizer::get_token_text(match));

    if ((match != -1) && next().match != match) {
        std::stringstream msg;
        msg << "Unexpected token (expected " << tokenizer::get_token_text(match)
            << ", found " << tokenizer::get_token_text(next().match)
            << " '" << next().text << "')";
        throw std::runtime_error(msg.str());
    }

    return tokens[currentIndex++].text;
}

tokenizer::Token const&
TokenStream::consumet()
{
    if (finished())
        // maybe we should handle this more gracefully
        throw std::runtime_error("Unexpected EOF");

    return tokens[currentIndex++];
}

bool
TokenStream::nextNonWhitespaceIs(int match, int lookahead) const
{
    return nextNonWhitespace(lookahead) == match;
}

int
TokenStream::getPosition() const
{
    return currentIndex;
}

void
TokenStream::resetPosition(int loc)
{
    assert(loc >= 0);
    currentIndex = loc;
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

