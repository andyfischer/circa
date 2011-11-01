// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "importing_macros.h"
#include "list_shared.h"

namespace circa {
namespace compound_type_tests {

void test_cast()
{
    Branch branch;
    Term* t = branch.compile("type T { string a }");
    TaggedValue* a = branch.eval("['hi']");
    test_assert(is_list(a));
    test_assert(a->value_type != unbox_type(t));

    TaggedValue casted;
    test_assert(cast(a, unbox_type(t), &casted));
    test_assert(is_list(&casted));
    test_assert(casted.value_type == unbox_type(t));
}

void test_bug_with_cast()
{
    // trying to repro a Plastic bug
    List value;
    set_int(value.append(), 1);
    set_int(value.append(), 72);
    set_int(value.append(), 18);

    test_equals(&value, "[1, 72, 18]");

    Branch branch;
    Term* type = branch.compile("type T {int x, int y, int z}");

    TaggedValue castResult;
    cast(&value, unbox_type(type), &castResult);

    test_equals(&castResult, "[1, 72, 18]");

    cast(&castResult, unbox_type(type), &castResult);

    test_equals(&castResult, "[1, 72, 18]");
}

void test_static_type_checking()
{
    Branch branch;
    Term* A = branch.compile("type A { int x, int y }");
    Term* B = branch.compile("type B { int x, any y }");
    Term* C = branch.compile("type C { int x, string y }");

    Term* a = branch.compile("A()");
    Term* b = branch.compile("B()");
    Term* c = branch.compile("C()");

    test_equals(run_static_type_query(unbox_type(A), a), StaticTypeQuery::SUCCEED);
    test_equals(run_static_type_query(unbox_type(A), b), StaticTypeQuery::UNABLE_TO_DETERMINE);
    test_equals(run_static_type_query(unbox_type(A), c), StaticTypeQuery::FAIL);

    test_equals(run_static_type_query(unbox_type(B), a), StaticTypeQuery::SUCCEED);
    test_equals(run_static_type_query(unbox_type(B), b), StaticTypeQuery::SUCCEED);
    test_equals(run_static_type_query(unbox_type(B), c), StaticTypeQuery::SUCCEED);

    test_equals(run_static_type_query(unbox_type(C), a), StaticTypeQuery::FAIL);
    test_equals(run_static_type_query(unbox_type(C), b), StaticTypeQuery::UNABLE_TO_DETERMINE);
    test_equals(run_static_type_query(unbox_type(C), c), StaticTypeQuery::SUCCEED);
}

void register_tests()
{
    REGISTER_TEST_CASE(compound_type_tests::test_cast);
    REGISTER_TEST_CASE(compound_type_tests::test_static_type_checking);
}

} // namespace compound_type_tests
} // namespace circa
