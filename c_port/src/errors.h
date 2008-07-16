#ifndef __ERRORS_INCLUDED__
#define __ERRORS_INCLUDED__

#include "common_headers.h"

class InternalError : public std::exception
{

};

class TypeError : public std::exception
{

};

#endif
