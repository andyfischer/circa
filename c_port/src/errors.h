#ifndef __ERRORS_INCLUDED__
#define __ERRORS_INCLUDED__

#include "common_headers.h"

namespace errors {

class CircaError : public std::exception
{
public:
    CircaError() throw() {}
    virtual string message() = 0;
};

class InternalError : public CircaError
{
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

    string msg;
};

class TypeError : public CircaError
{
public:
    virtual string message()
    {
        return string("Type error");
    }
};

}

#endif
