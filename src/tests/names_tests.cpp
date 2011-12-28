// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "filesystem_dummy.h"

namespace circa {
namespace names_tests {

void test_find_name()
{
    Branch branch;
    Term* a = create_int(&branch, 1, "a");

    test_assert(find_name(&branch, "a") == a);
    test_assert(find_name(&branch, "b") == NULL);

    Branch* sub = create_branch(&branch, "sub");
    test_assert(find_name(sub, "a") == a);

    Term* c = create_int(&branch, 3, "c");
    Term* another_a = create_int(&branch, 4, "a");

    // Check that find_name_at will ignore a second "a" definition
    test_assert(find_name(&branch, "a") == another_a);
    test_assert(find_name_at(c, "a") == a);
    test_assert(find_name(sub, "a") == a);
}

void test_name_is_reachable_from()
{
    Branch branch;

    Term* a = create_int(&branch, 5, "a");

    test_assert(name_is_reachable_from(a, &branch));

    Branch* sub_1 = create_branch(&branch, "sub_1");
    test_assert(name_is_reachable_from(a, sub_1));

    Branch* sub_2 = create_branch(sub_1, "sub_2");
    test_assert(name_is_reachable_from(a, sub_2));
}

void test_get_relative_name()
{
    Branch branch;
    Term* a = create_int(&branch, 5, "A");
    test_assert(get_relative_name(&branch, a) == "A");

    Branch* ns = create_namespace(&branch, "ns");
    test_assert(ns->owningTerm != NULL);
    Term* b = create_int(ns, 5, "B");

    test_assert(is_namespace(branch["ns"]));
    test_assert(ns->owningTerm != NULL);
    test_assert(ns->owningTerm == branch["ns"]);
    test_assert(get_relative_name(ns, b) == "B");
    test_equals(get_relative_name(&branch, b), "ns:B");

    // This code once had a bug:
    Term* c = branch.compile("[1 1] -> Point");
    test_assert(c->function->name == "cast");
    test_assert(c->type->name == "Point");
    //TEST_DISABLED test_equals(get_relative_name(c, c->type), "Point");
}

void test_get_relative_name_from_hidden_branch()
{
    // This code once had a bug
    Branch branch;
    branch.eval("if true { a = 1 } else { a = 2 }");

    test_equals(get_relative_name(&branch, branch.get("a")), "a");
}

void test_lookup_qualified_name()
{
    Branch branch;
    Term* a = branch.compile("namespace a { b = 1 }");
    Term* b = nested_contents(a)->get("b");

    test_assert(b != NULL);
    test_assert(branch["a:b"] == b);

    Term* x = branch.compile("namespace x { namespace y { namespace z { w = 1 }}}");
    Term* w = x->contents("y")->contents("z")->contents("w");
    test_assert(branch["x:y:z:w"] == w);
}

void test_get_named_at()
{
    Branch branch;

    // Simple case
    Term* a = create_int(&branch, 1, "a");
    Term* b = create_int(&branch, 1, "b");
    create_int(&branch, 1, "a");

    test_assert(get_named_at(b, "a") == a);

    // Make sure that the location term is not checked, should only check
    // things before that.
    Term* c1 = create_int(&branch, 1, "c");
    Term* c2 = create_int(&branch, 1, "c");
    create_int(&branch, 1, "c");
    test_assert(get_named_at(c2, "c") != c2);
    test_assert(get_named_at(c2, "c") == c1);

    // Find names in outer scopes
    Term* d = create_int(&branch, 1, "d");
    Branch* subBranch = create_branch(&branch);
    Term* e = create_int(subBranch, 1, "e");
    test_assert(get_named_at(e, "d") == d);

    // Make sure that when we look into outer scopes, we don't check the name of
    // our enclosing branch, or any names after that.
    Term* f1 = create_int(&branch, 1, "f");
    Branch* subBranch2 = create_branch(&branch, "f");
    Term* fsub = create_int(subBranch2, 1, "f");
    create_int(&branch, 1, "f");
    test_assert(get_named_at(fsub, "f") == f1);
}

void test_get_named_at_after_if_block()
{
    Branch branch;
    Term* originalA = branch.compile("a = 1");
    branch.compile("if true { a = 2 }");
    Term* b = branch.compile("b = 1");
    test_assert(branch["a"] != originalA); // sanity check
    test_assert(get_named_at(b, "a") != originalA);
    test_assert(get_named_at(b, "a") == branch["a"]);
}

void test_find_first_common_branch()
{
    Branch branch;
    Term* a = create_int(&branch, 0);
    Term* b = create_int(&branch, 0);

    test_assert(find_first_common_branch(a,b) == &branch);
    test_assert(find_first_common_branch(b,a) == &branch);

    Branch* b1 = create_branch(&branch);
    Term* c = create_int(b1, 0);
    Term* d = create_int(b1, 0);

    test_assert(find_first_common_branch(c,d) == b1);
    test_assert(find_first_common_branch(d,c) == b1);
    test_assert(find_first_common_branch(a,c) == &branch);
    test_assert(find_first_common_branch(d,b) == &branch);

    Branch alternate;
    Term* e = create_int(&alternate, 0);
    test_assert(find_first_common_branch(a,e) == NULL);
}

void test_unique_names()
{
    Branch branch;
    Term* a0 = create_int(&branch, 5, "a");
    Term* a1 = create_int(&branch, 5, "a");

    test_equals(a0->uniqueName.name, "a");
    test_assert(a0->uniqueName.ordinal == 0);
    test_equals(a1->uniqueName.name, "a_1");
    test_assert(a1->uniqueName.ordinal == 1);

    // Declare a name which overlaps with the next unique name that we'll
    // try to generate for 'a'.
    create_int(&branch, 5, "a_2");
    Term* a3 = create_int(&branch, 5, "a");

    test_equals(a3->uniqueName.name, "a_3");
    test_assert(a3->uniqueName.ordinal == 3);
}

void test_unique_names_for_anons()
{
    Branch branch;

    Term* a = branch.compile("add(1,2)");
    test_equals(a->uniqueName.name, "_add");

    Term* b = branch.compile("add(3,4)");
    test_equals(b->uniqueName.name, "_add_1");

    Term* c = branch.compile("_add_2 = add(3,4)");
    test_equals(c->uniqueName.name, "_add_2");

    Term* d = branch.compile("add(3,4)");
    test_equals(d->uniqueName.name, "_add_3");
}

void test_find_from_unique_name()
{
    Branch branch;
    Term* a0 = branch.compile("a = 0");
    Term* a1 = branch.compile("a = 1");
    Term* a2 = branch.compile("a = 2");

    test_assert(find_from_unique_name(&branch, "a") == a0);
    test_assert(find_from_unique_name(&branch, "a_1") == a1);
    test_assert(find_from_unique_name(&branch, "a_2") == a2);
    test_assert(find_from_unique_name(&branch, "b") == NULL);
}

void test_with_include()
{
    FakeFileSystem files;
    files["a"] = "namespace ns { a = 1 }";

    Branch branch;
    branch.compile("include('a')");
    Term* a = find_name(&branch, "ns:a");
    test_assert(a != NULL);

    test_assert(name_is_reachable_from(a, &branch));
}

void name_with_colons()
{
    Branch branch;
    Term* one = branch.compile("1");
    branch.bindName(one, "qualified:name");

    test_assert(branch["qualified:name"] == one);
    test_assert(find_name(&branch, "qualified:name") == one);

    Branch* ns = create_namespace(&branch, "ns");
    Term* two = ns->compile("2");
    ns->bindName(two, "another:name");

    test_assert(branch["ns:another:name"] == two);
    test_assert(find_name(&branch, "ns:another:name") == two);
}

void test_global_name()
{
    Branch* root = nested_contents(kernel()->get("_test_root"));

    Branch* branch = create_branch(root, "test_global_name");
    Term* a = branch->compile("a = 1");
    Term* b = branch->compile("if true { b = 3 }")->contents(0)->contents("b");

    std::string a_name;
    test_assert(find_global_name(a, a_name));
    test_equals(a_name, "_test_root:test_global_name:a");

    std::string b_name;
    test_assert(find_global_name(b, b_name));
    test_equals(b_name, "_test_root:test_global_name:_if_block:_case:b");

    test_assert(find_term_from_global_name(a_name.c_str()) == a);
    test_assert(find_term_from_global_name(b_name.c_str()) == b);
}

void test_global_name2()
{
    Branch branch;
    Term* a = branch.compile("a = 1");
    std::string str;
    test_assert(find_global_name(a, str) == false);

    test_assert(find_term_from_global_name("not:a:real:name") == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(names_tests::test_find_name);
    REGISTER_TEST_CASE(names_tests::test_name_is_reachable_from);
    REGISTER_TEST_CASE(names_tests::test_get_relative_name);
    REGISTER_TEST_CASE(names_tests::test_get_relative_name_from_hidden_branch);
    //TEST_DISABLED REGISTER_TEST_CASE(names_tests::test_lookup_qualified_name);
    REGISTER_TEST_CASE(names_tests::test_get_named_at);
    REGISTER_TEST_CASE(names_tests::test_get_named_at_after_if_block);
    REGISTER_TEST_CASE(names_tests::test_find_first_common_branch);
    REGISTER_TEST_CASE(names_tests::test_unique_names);
    REGISTER_TEST_CASE(names_tests::test_unique_names_for_anons);
    REGISTER_TEST_CASE(names_tests::test_find_from_unique_name);
    //TEST_DISABLED REGISTER_TEST_CASE(names_tests::test_with_include);
    REGISTER_TEST_CASE(names_tests::name_with_colons);
    REGISTER_TEST_CASE(names_tests::test_global_name);
    REGISTER_TEST_CASE(names_tests::test_global_name2);
}

} // namespace names_tests
} // namespace circa
