#ifndef __ERRORS_INCLUDED__
#define __ERRORS_INCLUDED__

#include "common_headers.h"

namespace circa {

namespace errors {

class CircaError : public std::exception
{
protected:
    std::string msg;
public:
    CircaError() throw() {}
    CircaError(std::string _msg) throw ()
    {
        this->msg = _msg;
    }
    ~CircaError() throw() {}
    virtual string message()
    {
        return this->msg;
    }
};

class InternalError : public CircaError
{
public:
    InternalError(string message) throw()
        : CircaError(message)
    {
    }
    ~InternalError() throw() {}

    virtual string message()
    {
        return string("Internal error: ") + this->msg;
    }
};

class TypeError : public CircaError
{
    Term* term;
    Term* expectedType;

public:
    TypeError(Term* _term, Term* _expectedType) throw()
        : CircaError("")
    {
        term = _term;
        expectedType = _expectedType;
    }
    ~TypeError() throw() {}
    virtual string message();
};

class KeyError : public CircaError
{
    string msg;

public:
    KeyError(string message) throw()
    {
        this->msg = message;
    }
    ~KeyError() throw() {}
    virtual string message()
    {
        return string("KeyError: ") + msg;
    }
};

class TokenMismatch : public CircaError
{
public:
    TokenMismatch(const char* expected, const char* found, const char* foundText) throw()
        : CircaError(std::string("Expected: ") + expected + ", found: " + found
               + " (" + foundText + ")")
    {
    }
};

} // namespace errors
} // namespace circa

#endif
