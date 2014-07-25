
// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "token.h"
#include "parser.h"

namespace parser_test {

void test_lookahead_anon_function()
{
    // Positive cases
    TokenStream tokens("-> blah");
    test_assert(parser::lookahead_match_anon_function(tokens));

    tokens.reset("() -> blah");
    test_assert(parser::lookahead_match_anon_function(tokens));

    tokens.reset("(a) -> blah");
    test_assert(parser::lookahead_match_anon_function(tokens));

    tokens.reset("(a,b) -> blah");
    test_assert(parser::lookahead_match_anon_function(tokens));

    tokens.reset("(   a,b) -> blah");
    test_assert(parser::lookahead_match_anon_function(tokens));

    tokens.reset("(a   ,b) -> blah");
    test_assert(parser::lookahead_match_anon_function(tokens));

    tokens.reset("(a,   b) -> blah");
    test_assert(parser::lookahead_match_anon_function(tokens));

    tokens.reset("(a,b   ) -> blah");
    test_assert(parser::lookahead_match_anon_function(tokens));

    // Negative cases
    tokens.reset("blah");
    test_assert(!parser::lookahead_match_anon_function(tokens));

    tokens.reset("()");
    test_assert(!parser::lookahead_match_anon_function(tokens));

    tokens.reset("(a)");
    test_assert(!parser::lookahead_match_anon_function(tokens));

    tokens.reset("(a,b)");
    test_assert(!parser::lookahead_match_anon_function(tokens));

    tokens.reset("(a,) ->");
    test_assert(!parser::lookahead_match_anon_function(tokens));

    tokens.reset("( ->");
    test_assert(!parser::lookahead_match_anon_function(tokens));
}

void register_tests()
{
    REGISTER_TEST_CASE(parser_test::test_lookahead_anon_function);
}

} // namespace register_tests
