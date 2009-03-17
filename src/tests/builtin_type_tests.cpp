// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "testing.h"
#include "cpp_importing.h"
#include "debug.h"
#include "builtins.h"
#include "branch.h"
#include "runtime.h"

namespace circa {
namespace builtin_type_tests {

void test_reference()
{
    Branch branch;
    Term *r = branch.eval("r = Ref()");
    Term *s = branch.eval("s = 1.0");
    Term *t = branch.eval("t = 1.0");

    test_assert(as_ref(r) == NULL);
    as_ref(r) = s;
    test_assert(as_ref(r) == s);

    ReferenceIterator* it = start_reference_iterator(r);

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
    test_assert(as_type(REF_TYPE).alloc != NULL);
    test_assert(as_type(REF_TYPE).dealloc != NULL);
    //test_assert(as_type(FUNCTION_TYPE)->equals != NULL);
}

void reference_type_deletion_bug()
{
    // There used to be a bug where deleting a reference term would delete
    // the thing it was pointed to.
    Branch *branch = new Branch();

    Term* myref = apply(branch, REF_TYPE, RefList());

    myref->asRef() = INT_TYPE;

    delete branch;

    assert_good_pointer(INT_TYPE);
    test_assert(INT_TYPE->type != NULL);
}

void test_set()
{
    Branch branch;

    Term* s = branch.eval("s = Set()");

    test_assert(is_branch(s));
    test_assert(as_branch(s).numTerms() == 0);

    s = branch.eval("s.add(1)");
    test_assert(as_branch(s).numTerms() == 1);
    test_assert(as_branch(s)[0]->asInt() == 1);

    s = branch.eval("s.add(1)");
    test_assert(as_branch(s).numTerms() == 1);

    s = branch.eval("s.add(2)");
    test_assert(as_branch(s).numTerms() == 2);

    s = branch.eval("s.remove(1)");
    test_assert(as_branch(s).numTerms() == 1);
    test_assert(as_branch(s)[0]->asInt() == 2);

    // check that things are copied by value
    Term* val = branch.eval("val = 5");
    s = branch.eval("s.add(val)");

    test_assert(as_branch(s)[1]->asInt() == 5);
    as_int(val) = 6;
    test_assert(as_branch(s)[1]->asInt() == 5);
}

void test_list()
{
    Branch branch;

    Term* l = branch.eval("l = List()");

    test_assert(is_branch(l));
    test_assert(as_branch(l).numTerms() == 0);

    l = branch.eval("l.append(2)");
    test_assert(as_branch(l).numTerms() == 1);
    test_assert(as_branch(l)[0]->asInt() == 2);
}

void register_tests()
{
    REGISTER_TEST_CASE(builtin_type_tests::test_reference);
    REGISTER_TEST_CASE(builtin_type_tests::builtin_types);
    REGISTER_TEST_CASE(builtin_type_tests::reference_type_deletion_bug);
    REGISTER_TEST_CASE(builtin_type_tests::test_set);
    REGISTER_TEST_CASE(builtin_type_tests::test_list);
}

} // namespace builtin_type_tests

} // namespace circa
