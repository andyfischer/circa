#ifndef CIRCA__TEST_COMMON__INCLUDED
#define CIRCA__TEST_COMMON__INCLUDED

#include "errors.h"

class TestFailure : public errors::CircaError
{
    virtual std::string message()
    {
        return "Test failure";
    }
};

void test_assert_f(bool condition, int line, const char* file);

#define test_assert(c) test_assert_f((c), __LINE__, __FILE__)


#endif
