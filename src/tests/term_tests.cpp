// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace term_tests {

void test_flags()
{
    Term term;

    test_assert(!term.hasError());
    term.setHasError(false);
    test_assert(!term.hasError());
    term.setHasError(true);
    test_assert(term.hasError());
}

void register_tests()
{
    REGISTER_TEST_CASE(term_tests::test_flags);
}

} // namespace term_tests
} // namespace circa
