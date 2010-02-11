// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

namespace circa {
namespace dynamic_type_tests {

void test_copy()
{
    Branch branch;
    Term* a = branch.compile("a = 5");
    Term* copy = branch.compile("copy(a)");
    evaluate_branch(branch);

    test_assert(is_int(copy));

    change_type(a, STRING_TYPE);
    set_str(a, "hi");
    evaluate_branch(branch);
    test_assert(is_string(copy));
}

void register_tests()
{
    test_copy();
}

} // namespace dynamic_type_tests
} // namespace circa
