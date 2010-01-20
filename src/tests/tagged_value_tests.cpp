// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace tagged_value_tests {

void test_int_simple()
{
    TaggedValue v;
    set_int(v, 4);

    test_assert(is_value_int(v));
    test_assert(as_int(v) == 4);

    Branch branch;
    Term* a = branch.eval("a = 1");
    test_assert(is_value_int(a));

    Term* b = branch.eval("b = a");
    test_assert(is_value_int(b));
}

void test_polymorphic()
{
    TaggedValue v;
    test_assert(!is_value_int(v));
    test_assert(!is_value_float(v));
    test_assert(!is_value_bool(v));

    set_int(v, 11);
    test_assert(is_value_int(v));
    test_assert(!is_value_float(v));
    test_assert(!is_value_bool(v));

    set_float(v, 2.0);
    test_assert(!is_value_int(v));
    test_assert(is_value_float(v));
    test_assert(!is_value_bool(v));

    set_bool(v, false);
    test_assert(!is_value_int(v));
    test_assert(!is_value_float(v));
    test_assert(is_value_bool(v));
}

void test_term_value()
{
    Branch branch;
    Term* i = create_int(branch, 5);
    test_assert(is_int(i));
    test_assert(is_value_int(i->value));

    Term* a = branch.eval("a = [1 2 3]");
    test_assert(is_value_int(a->asBranch()[0]));

    Term* b = branch.eval("b = a");
    test_assert(is_value_int(b->asBranch()[0]));
    
    Term* c = create_value(branch, INT_TYPE);
    test_assert(is_value_int(c));

    Branch list;
    resize_list(list, 4, INT_TYPE);
    test_assert(is_value_int(list[0]));
}

namespace subroutine_call_test_helper {
    void assert_ints(Term* term)
    {
        test_assert(is_value_int(term->input(0)));
        test_assert(is_value_int(term));
    }
}

void subroutine_call_test()
{
    Branch branch;
    import_function(branch, subroutine_call_test_helper::assert_ints, "f(int) -> int");
    branch.eval("f(1)");
}

void test_float()
{
    TaggedValue f = tag_float(5);
    TaggedValue i = tag_int(5);
    test_assert(is_value_int(i));
    test_assert(is_value_float(f));
    assign_value(i, f);
    test_assert(is_value_float(f));
}

void test_assign_value_to_default()
{
    Branch branch;
    Term* a = create_int(branch, 5);
    test_assert(as_int(a) == 5);
    assign_value_to_default(a);
    test_assert(as_int(a) == 0);
}

void test_constructor_syntax()
{
    Branch branch;
    Type myType;
    myType.name = "T";
    import_type(branch, &myType);
    Term* a = branch.eval("a = T()");
    test_assert(a->value.type == &myType);
    test_assert(a->value.data.ptr == NULL);
    assign_value_to_default(a);
    test_assert(a->value.type == &myType);
    test_assert(a->value.data.ptr == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(tagged_value_tests::test_int_simple);
    REGISTER_TEST_CASE(tagged_value_tests::test_polymorphic);
    REGISTER_TEST_CASE(tagged_value_tests::test_term_value);
    REGISTER_TEST_CASE(tagged_value_tests::subroutine_call_test);
    REGISTER_TEST_CASE(tagged_value_tests::test_float);
    REGISTER_TEST_CASE(tagged_value_tests::test_assign_value_to_default);
    REGISTER_TEST_CASE(tagged_value_tests::test_constructor_syntax);
}

}
}
