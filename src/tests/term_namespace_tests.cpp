// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace term_namespace_tests {

void test_append()
{
    TermNamespace a;
    TermNamespace b;

    Ref x(new Term), y(new Term), y2(new Term), z(new Term);

    a["x"] = x;
    a["y"] = y;

    b["y"] = y2;
    b["z"] = z;

    test_assert(a["x"] == x);
    test_assert(a["y"] == y);
    test_assert(a["z"] == NULL);

    a.append(b);

    test_assert(a["x"] == x);
    test_assert(a["y"] == y2);
    test_assert(a["z"] == z);
}

void register_tests()
{
    REGISTER_TEST_CASE(term_namespace_tests::test_append);
}

}
} // namespace circa
