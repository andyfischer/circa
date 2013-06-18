// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "building.h"
#include "kernel.h"
#include "term.h"
#include "type.h"
#include "tagged_value.h"

namespace type_test {

void reference_counting()
{
    Type* type = create_type();
    test_assert(type->header.refcount == 1);

    Value value;
    set_type(&value, type);
    test_assert(type->header.refcount == 2);

    set_null(&value);
    test_assert(type->header.refcount == 1);

    {
        Block block;
        Term* value = create_value(&block, type, "my_value");
        test_assert(type->header.refcount == 3);

        set_null(term_value(value));
        test_assert(type->header.refcount == 2);
    }

    test_assert(type->header.refcount == 1);

    set_type(&value, type);
    test_assert(type->header.refcount == 2);

    Value value2;
    copy(&value, &value2);
    test_assert(type->header.refcount == 3);

    copy(&value, &value2);
    test_assert(type->header.refcount == 3);
    
    type_decref(type);
}

void reference_counting_and_tagged_value()
{
    Type* type = create_type();
    test_assert(type->header.refcount == 1);

    Value value;
    make(type, &value);
    test_assert(type->header.refcount == 2);

    Value value2;
    copy(&value, &value2);
    test_assert(type->header.refcount == 3);
    copy(&value, &value2);
    test_assert(type->header.refcount == 3);

    set_null(&value2);
    test_assert(type->header.refcount == 2);

    type_decref(type);
}

void register_tests()
{
    REGISTER_TEST_CASE(type_test::reference_counting);
    REGISTER_TEST_CASE(type_test::reference_counting_and_tagged_value);
}

} // namespace type_test
