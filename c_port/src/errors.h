#ifndef __ERRORS_INCLUDED__
#define __ERRORS_INCLUDED__

#include "common_headers.h"

class Term;

namespace errors {

class CircaError : public std::exception
{
public:
    CircaError() throw() {}
    virtual string message() = 0;
};

class InternalError : public CircaError
{
protected:
    string msg;

public:
    InternalError(string message) throw()
    {
        this->msg = message;
    }
    ~InternalError() throw() {}

    virtual string message()
    {
        return string("Internal error: ") + this->msg;
    }
};

class InternalTypeError : public InternalError
{
    Term* term;
    Term* expectedType;

public:
    InternalTypeError(Term* _term, Term* _expectedType) throw()
        : InternalError("")
    {
        term = _term;
        expectedType = _expectedType;
    }
    ~InternalTypeError() throw() {}
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

}

#endif
