// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "weak_ref.h"
#include <circa.h>

namespace circa {
namespace weak_ref_tests {

struct MyStruct
{
    WeakRefOwner<MyStruct> _weakRefOwner;

    MyStruct() {
        _weakRefOwner.initialize(this);
    }
};

void simple()
{
    WeakRef<MyStruct> ref;
    test_assert(*ref == NULL);

    {
        MyStruct myStruct;
        ref = &myStruct;

        test_assert(ref == &myStruct);
    }

    test_assert(*ref == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(weak_ref_tests::simple);
}

} // namespace weak_ref_tests
} // namespace circa
