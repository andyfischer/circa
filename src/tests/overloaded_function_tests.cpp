// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace overloaded_function_tests {

void declared_in_script()
{
    Branch branch;
    branch.compile("def f_1(int i) -> int return i + 1 end");
    branch.compile("def f_2(string s) -> string return concat(s 'x') end");
    branch.compile("f = overloaded_function(f_1 f_2)");
    test_assert(branch);

    EvalContext context;
    evaluate_branch(&context, branch);
    test_assert(context);

    TaggedValue* a = branch.eval("a = f(1)");
    test_equals(a, 2);
    TaggedValue* b = branch.eval("b = f('aaa')");
    test_equals(b, "aaax");
}

void register_tests()
{
    REGISTER_TEST_CASE(overloaded_function_tests::declared_in_script);
}

} // namespace overloaded_function_tests
} // namespace circa
