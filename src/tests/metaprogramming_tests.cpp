// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace metaprogramming_tests {

void test_lift_closure()
{
    Branch branch;
    Term* a = branch.eval("a = 1");
    Branch& sub = create_branch(branch);
    Term* sub_a = sub.eval("a");
    Term* sub_freeze_a = sub.eval("freeze(a)");

    test_assert(sub_a->input(0) == a);
    test_assert(as_int(sub_a) == 1);
    test_assert(sub_freeze_a->input(0) == a);
    test_assert(as_int(sub_a) == 1);

    lift_closure(sub);
    make_int(a, 2);
    evaluate_branch(branch);

    test_assert(sub_a->input(0) == a);
    test_assert(as_int(sub_a) == 2);
    test_assert(sub_freeze_a->numInputs() == 0);
    test_assert(as_int(sub_freeze_a) == 1);
}

void save_code_generated_with_reflection()
{
    FakeFileSystem files;
    files["included.ca"] = "";

    Branch branch;
    Term* b = branch.eval("b = include('included.ca')");
    test_equals(b->nestedContents["#attr:source-file"]->asString(), "included.ca");

    branch.eval("bm = branch_ref(b)");
    branch.eval("def my_function() end");

    test_assert(b->nestedContents.contains("#attr:source-file"));
    test_equals(b->nestedContents["#attr:source-file"]->asString(), "included.ca");

    branch.eval("bm.append_code({my_function()})");

    // there was once a bug here:
    test_assert(b->nestedContents.contains("#attr:source-file"));
    test_equals(b->nestedContents["#attr:source-file"]->asString(), "included.ca");

    branch.eval("bm.append_code({my_function()})");
    branch.eval("bm.append_code({my_function()})");
    branch.eval("bm.save()");

    test_equals(files["included.ca"], "my_function()\nmy_function()\nmy_function()\n");
}

void register_tests()
{
    REGISTER_TEST_CASE(metaprogramming_tests::test_lift_closure);
    REGISTER_TEST_CASE(metaprogramming_tests::save_code_generated_with_reflection);
}

} // namespace metaprogramming_tests
} // namespace circa
