// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "testing.h"
#include "cpp_importing.h"
#include "debug.h"
#include "builtins.h"
#include "branch.h"
#include "parser.h"
#include "runtime.h"

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
    Term *r = branch.eval("r = Ref()");
    Term *s = branch.eval("s = 1.0");
    Term *t = branch.eval("t = 1.0");

    test_assert(as_ref(r) == NULL);
    as_ref(r) = s;
    test_assert(as_ref(r) == s);

    PointerIterator* it = start_pointer_iterator(r);

    test_assert(it->current() == s);

    as_ref(r) = t;

    test_assert(it->current() == t);

    it->advance();
    test_assert(it->finished());

    delete it;
}

void builtin_types()
{
    test_assert(as_type(INT_TYPE).alloc != NULL);
    test_assert(as_type(INT_TYPE).dealloc != NULL);
    test_assert(as_type(INT_TYPE).equals != NULL);
    test_assert(as_type(FLOAT_TYPE).alloc != NULL);
    test_assert(as_type(FLOAT_TYPE).dealloc != NULL);
    test_assert(as_type(FLOAT_TYPE).equals != NULL);
    test_assert(as_type(STRING_TYPE).alloc != NULL);
    test_assert(as_type(STRING_TYPE).dealloc != NULL);
    test_assert(as_type(STRING_TYPE).equals != NULL);
    test_assert(as_type(BOOL_TYPE).alloc != NULL);
    test_assert(as_type(BOOL_TYPE).dealloc != NULL);
    test_assert(as_type(BOOL_TYPE).equals != NULL);
    test_assert(as_type(TYPE_TYPE).alloc != NULL);
    test_assert(as_type(TYPE_TYPE).dealloc != NULL);
    //test_assert(as_type(TYPE_TYPE)->equals != NULL);
    test_assert(as_type(FUNCTION_TYPE).alloc != NULL);
    test_assert(as_type(FUNCTION_TYPE).dealloc != NULL);
    test_assert(as_type(REFERENCE_TYPE).alloc != NULL);
    test_assert(as_type(REFERENCE_TYPE).dealloc != NULL);
    //test_assert(as_type(FUNCTION_TYPE)->equals != NULL);
}

void reference_type_deletion_bug()
{
    // There used to be a bug where deleting a reference term would delete
    // the thing it was pointed to.
    Branch *branch = new Branch();

    Term* myref = apply_function(branch, REFERENCE_TYPE, ReferenceList());

    myref->asRef() = INT_TYPE;

    delete branch;

    assert_good_pointer(INT_TYPE);
    test_assert(INT_TYPE->type != NULL);
}


void register_tests()
{
    REGISTER_TEST_CASE(builtin_type_tests::test_dictionary);
    REGISTER_TEST_CASE(builtin_type_tests::test_reference);
    REGISTER_TEST_CASE(builtin_type_tests::builtin_types);
    REGISTER_TEST_CASE(builtin_type_tests::reference_type_deletion_bug);
}

} // namespace builtin_type_tests

} // namespace circa
