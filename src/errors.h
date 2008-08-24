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
    Term* offendingTerm;
    Term* expectedType;

public:
    TypeError(Term* _offendingTerm, Term* _expectedType) throw()
        : CircaError("")
    {
        offendingTerm = _offendingTerm;
        expectedType = _expectedType;
    }
    ~TypeError() throw() {}
    virtual string message();
};

class KeyError : public CircaError
{
public:
    KeyError(string message) throw()
      : CircaError(message)
    {
    }
    ~KeyError() throw() {}
    virtual string message()
    {
        return string("KeyError: ") + this->msg;
    }
};

} // namespace errors
} // namespace circa

#endif
