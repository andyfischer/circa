// Copyright 2008 Andrew Fischer

#ifndef CIRCA_ERRORS_INCLUDED
#define CIRCA_ERRORS_INCLUDED

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
    virtual std::string message()
    {
        return this->msg;
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
    virtual std::string message();
};

} // namespace errors
} // namespace circa

#endif
