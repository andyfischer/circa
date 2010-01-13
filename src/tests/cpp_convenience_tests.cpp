// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace cpp_convenience_tests {

#if 0
void test_accessor()
{
    Branch branch;
    Int a(branch, "a", 5);

    test_assert(a == 5);
    test_assert(branch.contains("a"));
    test_assert(branch["a"]->asInt() == 5);
    a = 3;
    test_assert(branch["a"]->asInt() == 3);

    Term* b = create_value(branch, INT_TYPE, "b");
    set_value_int(b, 8);
    Int b_accessor(branch, "b", 3);
    test_assert(b_accessor == 8);
    test_assert(as_int(b) == 8);

    b_accessor = 11;
    test_assert(b_accessor == 11);
    test_assert(as_int(b) == 11);
}
#endif

void test_as()
{
    Branch branch;
    branch.eval("i = 5");
    branch.eval("f = 11.0");
    branch.eval("b = true");

    //test_assert(as<int>(branch["i"]) == 5);
    //test_assert(as<float>(branch["f"]) == 11.0);
    //test_assert(as<bool>(branch["b"]));
}

void test_eval()
{
    test_assert(eval<int>("1 + 1") == 2);
    test_equals(eval<float>("6 * 1.5"), 9);
    test_assert(!eval<bool>("and(true,false)"));
    test_assert(eval<bool>("or(true,false)"));
    test_assert(eval<std::string>("concat('123','456')") == "123456");
}

void register_tests()
{
    //REGISTER_TEST_CASE(cpp_convenience_tests::test_accessor);
    //REGISTER_TEST_CASE(cpp_convenience_tests::test_as);
    //REGISTER_TEST_CASE(cpp_convenience_tests::test_eval);
}

}
} // namespace circa
