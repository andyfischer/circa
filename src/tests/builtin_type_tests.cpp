// Copyright 2008 Andrew Fischer

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

void test_reference()
{
    Branch branch;
    Term *r = branch.eval("r = Reference()");
    Term *s = branch.eval("s = 1.0");
    Term *t = branch.eval("t = 1.0");

    test_assert(as_ref(r) == NULL);
    as_ref(r) = s;
    test_assert(as_ref(r) == s);

    PointerIterator* it = start_pointer_iterator(r);

    test_assert(it->current() == s);

    it->current() = t;

    test_assert(as_ref(r) == t);

    it->advance();
    test_assert(it->finished());

    delete it;
}

void register_tests()
{
    REGISTER_TEST_CASE(builtin_type_tests::test_dictionary);
    REGISTER_TEST_CASE(builtin_type_tests::test_reference);
}

} // namespace builtin_type_tests

} // namespace circa
