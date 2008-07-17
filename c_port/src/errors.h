#ifndef __ERRORS_INCLUDED__
#define __ERRORS_INCLUDED__

#include "common_headers.h"

namespace errors {

class CircaError : public std::exception
{
};

class InternalError : public CircaError
{
public:
    InternalError(string message) throw()
    {
        this->msg = message;
    }
    virtual ~InternalError() throw() {}

    virtual const char* what()
    {
        return this->msg.c_str();
    }

    string msg;
};

class TypeError : public CircaError
{
};

}

#endif
