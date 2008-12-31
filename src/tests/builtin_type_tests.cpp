// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "testing.h"
#include "cpp_interface.h"
#include "builtins.h"
#include "branch.h"
#include "parser.h"

namespace circa {
namespace builtin_type_tests {

void test_dictionary()
{
    Branch branch;
    Term *d = branch.eval("d = Dictionary()");
    test_assert(d != NULL);
    as<Dictionary>(d);

    // todo: add more here
}

} // namespace builtin_type_tests

void register_builtin_type_tests()
{
    REGISTER_TEST_CASE(builtin_type_tests::test_dictionary);
}

} // namespace circa
