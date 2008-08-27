#ifndef CIRCA__SYNTAX_ERRORS__INCLUDED
#define CIRCA__SYNTAX_ERRORS__INCLUDED

#include <exception>

#include "errors.h"

namespace circa {

namespace tokenizer { class TokenInstance; }

namespace syntax_errors {

class SyntaxError : public circa::errors::CircaError
{
public:
    SyntaxError(std::string const& message,
            tokenizer::TokenInstance const* location = NULL) throw();
};

class UnexpectedToken : public SyntaxError
{
public:
    UnexpectedToken(int expected, int found, std::string const& foundText) throw();
};

} // namespace syntax_errors
} // namespace circa

#endif
