// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace cpp_importing_tests {

struct TypeWithUninitializedValue
{
    int value;
};

void test_zeroed_memory()
{
    Branch branch;
    Term* t = import_type<TypeWithUninitializedValue>(branch, "T");

    // Mess up some memory, to increase the chances that this type will use some
    // dirty memory. On my Mac this doesn't really work, but it's worth a shot.
    void* blah = malloc(4096);
    memset(blah, 0xffffffff, 4096);
    free(blah);

    Term* v = create_value(branch, t);
    test_assert(as<TypeWithUninitializedValue>(v).value == 0);
}

struct TypeWithSimpleConstructor
{
    int value;
    TypeWithSimpleConstructor() : value(5) {}
};

void test_constructor()
{
    Branch branch;

    Term* t = import_type<TypeWithSimpleConstructor>(branch, "T");
    Term* v = create_value(branch, t);

    test_assert(as<TypeWithSimpleConstructor>(v).value == 5);
}

void register_tests()
{
    REGISTER_TEST_CASE(cpp_importing_tests::test_zeroed_memory);
    REGISTER_TEST_CASE(cpp_importing_tests::test_constructor);
}

} // namespace cpp_importing_tests

} // namespace circa
