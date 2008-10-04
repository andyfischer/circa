// Copyright 2008 Paul Hodge

#ifndef CIRCA__ERRORS__INCLUDED
#define CIRCA__ERRORS__INCLUDED

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

} // namespace errors
} // namespace circa

#endif
