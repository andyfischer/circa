#ifndef CIRCA__SYNTAX_ERRORS__INCLUDED
#define CIRCA__SYNTAX_ERRORS__INCLUDED

#include <exception>

#include "errors.h"

namespace circa {
namespace syntax_errors {

class ParseError : public circa::errors::CircaError
{
public:
    ParseError(std::string const& message) throw()
      : circa::errors::CircaError(message)
    {
    }
};

class UnexpectedToken : public ParseError
{
public:
    UnexpectedToken(const char* expected, const char* found, const char* foundText) throw()
        : ParseError(std::string("Expected: ") + expected + ", found: " + found
               + " (" + foundText + ")")
    {
    }
};

class UnexpectedEOF : public ParseError
{
public:
    UnexpectedEOF() throw()
      : ParseError("Unexpected EOF")
    {
    }
};

} // namespace syntax_errors
} // namespace circa

#endif
