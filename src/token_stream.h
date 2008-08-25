#ifndef CIRCA__TOKEN_STREAM__INCLUDED
#define CIRCA__TOKEN_STREAM__INCLUDED

#include <set>

#include "syntax_errors.h"
#include "tokenizer.h"

namespace circa {
namespace token_stream {

struct TokenStream
{
    tokenizer::TokenList tokens;
    int currentIndex;
    std::set<const char*> skipSet;

    TokenStream(tokenizer::TokenList const& _tokens)
      : tokens(_tokens), currentIndex(0)
    {
    }

    TokenStream(std::string const& input)
      : tokens(tokenizer::tokenize(input)), currentIndex(0)
    {
    }

    /*
    bool shouldSkip(const char* token)
    {
        return this->skipSet.find(token) != this->skipSet.end();
    }

    int indexAfterSkipping(int index)
    {
        while (true)
        {
            if (this->finished())
                return index;

            if (shouldSkip(tokens[index])) {
                index++;
            } else {
                return index;
            }
        }
    }
    */

    void stripWhitespace()
    {
        int deleteCount = 0;

        for (int i=0; i < tokens.size(); i++) {

            if (tokens[i].match == tokenizer::WHITESPACE) {
                deleteCount++;
            } else if (deleteCount > 0) {
                tokens[i - deleteCount] = tokens[i];
            }
        }

        tokens.resize(tokens.size() - deleteCount);
    }

    tokenizer::TokenInstance const& next(int lookahead=0) const
    {
        int i = currentIndex + lookahead;

        if (i >= tokens.size())
            throw syntax_errors::UnexpectedEOF();

        return tokens[i];
    }

    bool nextIs(const char * match, int lookahead=0) const
    {
        return next(lookahead).match == match;
    }

    std::string consume(const char * match = NULL)
    {
        if (finished())
            throw syntax_errors::UnexpectedEOF();

        if ((match != NULL) && next().match != match)
            throw syntax_errors::UnexpectedToken(match, next().match, next().text.c_str());

        return tokens[currentIndex++].text;
    }

    bool finished() const
    {
        return (currentIndex >= tokens.size());
    }
};

} // namespace token_stream
} // namespace circa

#endif
