// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace parser_util_tests {

void test_get_number_of_decimal_figures()
{
    test_assert(get_number_of_decimal_figures("1") == 0);
    test_assert(get_number_of_decimal_figures("9438432") == 0);
    test_assert(get_number_of_decimal_figures("1.") == 1);
    test_assert(get_number_of_decimal_figures("1.1") == 1);
    test_assert(get_number_of_decimal_figures("1.10") == 2);
    test_assert(get_number_of_decimal_figures(".10") == 2);
    test_assert(get_number_of_decimal_figures("0.101010") == 6);
}

void register_tests()
{
    REGISTER_TEST_CASE(parser_util_tests::test_get_number_of_decimal_figures);
}

} // namespace parser_util_tests
} // namespace circa
