// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace float_tests {

void test_cast()
{
    TaggedValue f;
    make_float(&f, 5.0);
    TaggedValue s;
    make_string(&s, "hello");


    test_assert(cast_possible(&f, FLOAT_T));
    test_assert(!cast_possible(&s, FLOAT_T));

    TaggedValue f2;
    test_assert(cast2(&f, FLOAT_T, &f2));

    test_assert(equals(&f, &f2));
}

void register_tests()
{
    REGISTER_TEST_CASE(float_tests::test_cast);
}

} // namespace float_tests
} // namespace circa
