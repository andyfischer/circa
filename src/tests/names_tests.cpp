// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace names_tests {

void test_find_named()
{
    Branch branch;
    Term* a = create_int(branch, 1, "a");

    test_assert(find_named(branch, "a") == a);
    test_assert(find_named(branch, "b") == NULL);

    Branch& sub = create_branch(branch, "sub");
    test_assert(find_named(sub, "a") == a);
}

void test_name_is_reachable_from()
{
    Branch branch;

    Term* a = create_int(branch, 5, "a");

    test_assert(name_is_reachable_from(a, branch));

    Branch& sub_1 = create_branch(branch, "sub_1");
    test_assert(name_is_reachable_from(a, sub_1));

    Branch& sub_2 = create_branch(sub_1, "sub_2");
    test_assert(name_is_reachable_from(a, sub_2));
}

void test_get_relative_name()
{
    Branch branch;
    Term* a = create_int(branch, 5, "A");
    test_assert(get_relative_name(branch, a) == "A");

    Branch& ns = create_namespace(branch, "ns");
    test_assert(ns.owningTerm != NULL);
    Term* b = create_int(ns, 5, "B");

    test_assert(is_namespace(branch["ns"]));
    test_assert(ns.owningTerm != NULL);
    test_assert(ns.owningTerm == branch["ns"]);
    test_assert(get_relative_name(ns, b) == "B");
    test_equals(get_relative_name(branch, b), "ns:B");

    // This code once had a bug:
    Term* c = branch.eval("[1 1] -> Point");
    test_assert(c->function->name == "cast");
    test_assert(c->type->name == "Point");
    test_equals(get_relative_name(c, c->type), "Point");
}

void test_get_relative_name_from_hidden_branch()
{
    // This code once had a bug
    Branch branch;
    branch.eval("if true; a = 1; else; a = 2; end");

    test_equals(get_relative_name(branch, branch["a"]), "a");
}

void test_lookup_qualified_name()
{
    Branch branch;
    Term* a = branch.eval("namespace a { b = 1 }");
    Term* b = a->nestedContents["b"];

    test_assert(b != NULL);
    test_assert(branch["a:b"] == b);

    Term* x = branch.eval("namespace x { namespace y { namespace z { w = 1 }}}");
    Term* w = x->nestedContents["y"]->nestedContents["z"]->nestedContents["w"];
    test_assert(branch["x:y:z:w"] == w);
}

void test_get_named_at()
{
    Branch branch;

    // Simple case
    Term* a = create_int(branch, 1, "a");
    Term* b = create_int(branch, 1, "b");
    create_int(branch, 1, "a");

    test_assert(get_named_at(b, "a") == a);

    // Make sure that the location term is not checked, should only check
    // things before that.
    Term* c1 = create_int(branch, 1, "c");
    Term* c2 = create_int(branch, 1, "c");
    create_int(branch, 1, "c");
    test_assert(get_named_at(c2, "c") != c2);
    test_assert(get_named_at(c2, "c") == c1);

    // Find names in outer scopes
    Term* d = create_int(branch, 1, "d");
    Branch& subBranch = create_branch(branch);
    Term* e = create_int(subBranch, 1, "e");
    test_assert(get_named_at(e, "d") == d);

    // Make sure that when we look into outer scopes, we don't check the name of
    // our enclosing branch, or any names after that.
    Term* f1 = create_int(branch, 1, "f");
    Branch& subBranch2 = create_branch(branch, "f");
    Term* fsub = create_int(subBranch2, 1, "f");
    create_int(branch, 1, "f");
    test_assert(get_named_at(fsub, "f") == f1);

    // Find names in exposed-name branches
    Branch& subBranch3 = create_branch(branch);
    Term* h = create_int(subBranch3, 1, "h");
    subBranch3.owningTerm->setBoolProp("exposesNames", true);
    Term* i = create_int(branch, 1, "i");
    test_assert(get_named_at(i, "h") == h);
}

void test_find_first_common_branch()
{
    Branch branch;
    Term* a = create_int(branch, 0);
    Term* b = create_int(branch, 0);

    test_assert(find_first_common_branch(a,b) == &branch);
    test_assert(find_first_common_branch(b,a) == &branch);

    Branch& b1 = create_branch(branch);
    Term* c = create_int(b1, 0);
    Term* d = create_int(b1, 0);

    test_assert(find_first_common_branch(c,d) == &b1);
    test_assert(find_first_common_branch(d,c) == &b1);
    test_assert(find_first_common_branch(a,c) == &branch);
    test_assert(find_first_common_branch(d,b) == &branch);

    Branch alternate;
    Term* e = create_int(alternate, 0);
    test_assert(find_first_common_branch(a,e) == NULL);
}

void test_unique_names()
{
    Branch branch;
    Term* a0 = create_int(branch, 5, "a");
    Term* a1 = create_int(branch, 5, "a");

    test_equals(a0->uniqueName.name, "a");
    test_assert(a0->uniqueName.ordinal == 0);
    test_equals(a1->uniqueName.name, "a_1");
    test_assert(a1->uniqueName.ordinal == 1);

    // Declare a name which overlaps with the next unique name that we'll
    // try to generate for 'a'.
    create_int(branch, 5, "a_2");
    Term* a3 = create_int(branch, 5, "a");

    test_equals(a3->uniqueName.name, "a_3");
    test_assert(a3->uniqueName.ordinal == 3);
}

void register_tests()
{
    REGISTER_TEST_CASE(names_tests::test_find_named);
    REGISTER_TEST_CASE(names_tests::test_name_is_reachable_from);
    REGISTER_TEST_CASE(names_tests::test_get_relative_name);
    REGISTER_TEST_CASE(names_tests::test_get_relative_name_from_hidden_branch);
    REGISTER_TEST_CASE(names_tests::test_lookup_qualified_name);
    REGISTER_TEST_CASE(names_tests::test_get_named_at);
    REGISTER_TEST_CASE(names_tests::test_find_first_common_branch);
    REGISTER_TEST_CASE(names_tests::test_unique_names);
}

} // namespace names_tests
} // namespace circa
