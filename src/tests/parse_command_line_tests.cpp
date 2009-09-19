// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace parse_command_line_tests {

void test_simple()
{
    CommandLineParser parser;

    test_assert(parser._findOption("what") == -1);

    parser.expectOption("-a", 0);
    parser.expectOption("-b", 1);

    test_assert(parser._findOption("-a") != -1);
    test_assert(parser._findOption("-b") != -1);

    // now try to actually parse something
    parser.parse("-a -b 1 2 3");

    test_assert(parser.found("-a")); 
    test_assert(parser.found("-b")); 
    test_assert(!parser.found("-c")); 
    test_assert(parser.getParam("-b", 0) == "1");
    test_assert(parser.remainingArgs.size() == 2);
    test_assert(parser.remainingArgs[0] == "2");
    test_assert(parser.remainingArgs[1] == "3");
}

void register_tests()
{
    REGISTER_TEST_CASE(parse_command_line_tests::test_simple);
}

} // namespace parse_command_line_tests
} // namespace circa
