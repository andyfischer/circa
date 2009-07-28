// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace utils_tests {

void test_get_directory_for_filename()
{
    test_equals(get_directory_for_filename("c:/My Documents/settings.txt"),
            "c:/My Documents");
    test_equals(get_directory_for_filename("settings.txt"), ".");
    test_equals(get_directory_for_filename("/settings.txt"), "/");
}

void register_tests()
{
    REGISTER_TEST_CASE(utils_tests::test_get_directory_for_filename);
}

} // namespace utils_tests
} // namespace circa
