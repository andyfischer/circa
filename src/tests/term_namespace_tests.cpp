// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace term_namespace_tests {

void test_append()
{
    TermNamespace a;
    TermNamespace b;

    Ref x = alloc_term();
    Ref y = alloc_term();
    Ref y2 = alloc_term();
    Ref z = alloc_term();

    test_assert(x->refCount == 1);
    test_assert(y->refCount == 1);
    test_assert(y2->refCount == 1);
    test_assert(z->refCount == 1);

    a["x"] = x;

    test_assert(x->refCount == 2);
    test_assert(y->refCount == 1);
    test_assert(y2->refCount == 1);
    test_assert(z->refCount == 1);

    a["y"] = y;

    test_assert(x->refCount == 2);
    test_assert(y->refCount == 2);
    test_assert(y2->refCount == 1);
    test_assert(z->refCount == 1);

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
