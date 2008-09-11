// Copyright 2008 Andrew Fischer

#include "common.h"

namespace circa {

std::vector<TestCase> gTestCases;

void _test_assert_function(bool condition, int line, const char* file)
{
    if (!condition) {
        std::stringstream msg;
        msg << "Assert failure in " << file << ", line: " << line;
        throw errors::CircaError(msg.str());
    }
}

void _test_fail_function(int line, const char* file)
{
    std::stringstream msg;
    msg << "Test fail in " << file << ", line: " << line;
    throw errors::CircaError(msg.str());
}

} // namespace circa
