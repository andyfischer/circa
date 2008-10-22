// Copyright 2008 Paul Hodge

#include "common.h"

namespace circa {

std::vector<TestCase> gTestCases;

void _test_assert_function(bool condition, int line, const char* file)
{
    if (!condition) {
        std::stringstream msg;
        msg << "Assert failure in " << file << ", line " << line;
        throw errors::CircaError(msg.str());
    }
}

void _test_fail_function(int line, const char* file)
{
    std::stringstream msg;
    msg << "Test fail in " << file << ", line " << line;
    throw errors::CircaError(msg.str());
}

void _test_equals_function(ReferenceList const& a, ReferenceList const& b,
        const char* aText, const char* bText, int line, const char* file)
{
    std::stringstream msg;

    if (a.count() != b.count()) {
        msg << "Equality fail in " << file << ", line " << line << std::endl;
        msg << "  " << aText << " has " << a.count() << " items, ";
        msg << bText << " has " << b.count() << " items.";
        throw errors::CircaError(msg.str());
    }

    for (unsigned int i=0; i < a.count(); i++) {
        if (a[i] != b[i]) {
            msg << "Equality fail in " << file << ", line " << line << std::endl;
            msg << "  " << aText << " != " << bText << " (index " << i << " differs)";
            throw errors::CircaError(msg.str());
        }
    }
}

} // namespace circa
