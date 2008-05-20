#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED

#include "CommonHeaders.h"

/*
class internal_error : public std::exception
{
public:
    internal_error(const string msg)
      : _message(msg)
    {
    }

    string _message;
};*/

void INTERNAL_ERROR(const string msg);

#endif
