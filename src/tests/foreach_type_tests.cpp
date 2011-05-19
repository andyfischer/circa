// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace foreach_type_tests {

bool run_test_for_type(Type* type, List& exampleValues)
{
    Branch branch;

    // Copy to x
    Value x;
    copy(exampleValues[0], &x);

    // Copy again to cpy, check they are equal
    Value cpy;
    copy(&x, &cpy);
    test_assert(equals(&x, &cpy));
    test_assert(equals(&cpy, &x));

    // Check if example 0 != example 1
    Value y;
    copy(exampleValues[1], &y);
    test_assert(!equals(&x,&y));
    test_assert(!equals(&y,&x));

    // Use 'equals' on a different type, check if we die
    Value nullValue;
    test_assert(!equals(&nullValue, &x));
    test_assert(!equals(&x, &nullValue));

    // Reset, check equality.
    reset(&x);
    reset(&y);
    test_assert(equals(&x,&y));

    // use cast(), make sure the output is equal
    Value castResult;
    cast(&x, type, &castResult);
    test_assert(equals(&x, &castResult));

    // do cast again, using same value for source and output
    cast(&castResult, type, &castResult);
    test_assert(equals(&x, &castResult));

    // Cast an integer to this type. This might cause an error but it shouldn't
    // crash.
    Value one;
    set_int(&one, 1);
    cast(&one, type, &castResult);

    return true;
}

void check_int()
{
    List intExamples;
    set_int(intExamples.append(), 5);
    set_int(intExamples.append(), 3);
    run_test_for_type(&INT_T, intExamples);
}

void check_float()
{
    List floatExamples;
    set_float(floatExamples.append(), 1.2);
    set_float(floatExamples.append(), -0.001);
    run_test_for_type(&FLOAT_T, floatExamples);
}

void check_string()
{
    List stringExamples;
    set_string(stringExamples.append(), "hello");
    set_string(stringExamples.append(), "goodbye");
    run_test_for_type(&STRING_T, stringExamples);
}

void check_ref()
{
    List refExamples;
    Term* refTarget1 = alloc_term();
    Term* refTarget2 = alloc_term();
    set_ref(refExamples.append(), refTarget1);
    set_ref(refExamples.append(), refTarget2);
    test_assert(run_test_for_type(&REF_T, refExamples));
}

void register_tests()
{
    REGISTER_TEST_CASE(foreach_type_tests::check_int);
    REGISTER_TEST_CASE(foreach_type_tests::check_float);
    REGISTER_TEST_CASE(foreach_type_tests::check_string);
    REGISTER_TEST_CASE(foreach_type_tests::check_ref);
}

}
} // namespace circa
